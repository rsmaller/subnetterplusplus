#include <iostream>
#include <vector>
#include <string.h>
#include <cmath>
#include <regex>
#include <libgen.h>
#define strdup

using namespace std;

typedef union IPNumeric {
    unsigned int IP32;
    unsigned char octets[4];
} IPNumeric;

typedef struct arguments {
    int IPArgumentIndex;
    bool IPArgumentFound = false;
    int netMask1ArgumentIndex;
    bool netMask1Found = false;
    int netMask2ArgumentIndex;
    bool netMask2Found = false;
    string customArgumentVector = "";
    bool customArgumentFound = false;
} arguments;

char **globalArgumentVector;
string programName;
arguments globalArgs;
bool binaryFlag = false;
bool debugFlag = false;
bool reverseFlag = false;
int numberFlag = 0;

void usage() {
    cout << "Usage: " << basename(globalArgumentVector[0]) << " ipaddr netmask1 <netmask2> <-b[inary]|d[ebug]|r[everse]|limitNumber>" << endl;
    exit(0);
}

class IP {
public:
    IPNumeric IPAddress;
    string IPString;
    string IPBinaryString;
    bool unusualFormat = false;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    IP(string stringArg) {
        if (isIPFormat(stringArg)) {
            this -> IPAddress = StringToIPInt(stringArg);
            this -> IPString = stringArg;
        } else if (isIPNumber(stringArg)) {
            this -> IPAddress.IP32 = (unsigned int)stoi(stringArg);
            this -> IPString = intToIPString(this -> IPAddress);
        } else {
            this -> IPString = "0.0.0.0";
            this -> IPAddress.IP32 = 0;
            this -> unusualFormat = true;
        }
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
    }

    IP(unsigned int IPArg) {
        this -> IPAddress.IP32 = IPArg;
        this -> IPString = intToIPString(this -> IPAddress);
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
    }

    IP() {} // necessary for prototypes to work.

    static bool isIPFormat(string testString) {
        smatch matches;
        regex IPPattern("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
        return regex_search(testString, matches, IPPattern);
    }

    static bool isCIDRMask(string testString) {
        smatch matches;
        regex CIDRPattern("^\\d{1,2}$");
        return regex_search(testString, matches, CIDRPattern) && (32 >= stoi(testString) && stoi(testString) >= 0);
    }

    static bool isIPNumber(string testString) {
        smatch matches;
        regex IPNumberPattern("^\\d{1,10}$");
        return regex_search(testString, matches, IPNumberPattern) && (testString == to_string((unsigned int)stoi(testString)));
    }

    static IPNumeric StringToIPInt(string stringArg) {
        IPNumeric returnValue;
        IPNumeric CIDRConversion;
        char *CStyleStringArg = (char *)malloc(sizeof(char) * stringArg.length() + 1);
        if (isIPFormat(stringArg)) {
            strcpy(CStyleStringArg, stringArg.c_str());
        } else if (isCIDRMask(stringArg)) {
            CIDRConversion.IP32 = ~((1<<(32-stoi(stringArg)))-1);
            strcpy(CStyleStringArg, intToIPString(CIDRConversion).c_str());
        } else {
            returnValue = {0};
            return returnValue;
        }
        char *currentOctet = strtok(CStyleStringArg, ".");
        returnValue.octets[3] = (unsigned char)atoi(currentOctet);
        currentOctet = strtok(NULL, ".");
        returnValue.octets[2] = (unsigned char)atoi(currentOctet);
        currentOctet = strtok(NULL, ".");
        returnValue.octets[1] = (unsigned char)atoi(currentOctet);
        currentOctet = strtok(NULL, ".");
        returnValue.octets[0] = (unsigned char)atoi(currentOctet);
        return returnValue;
    }

    static string intToIPString(IPNumeric IPArg) {
        string returnString = "";
        for (int i=3; i>0; i--) {
            returnString.append(to_string(IPArg.octets[i]));
            returnString.append(".");
        }
        returnString.append(to_string(IPArg.octets[0]));
        return returnString;
    }

    template <typename T>
    static string toBinaryString(T numArg) {
        string returnString = "";
        int maxBitSize = sizeof(T) * 8 - 1;
        for (int i=maxBitSize; i>=0; i--) {
            if (numArg - (1<<i) >=0) {
                numArg -= (1<<i);
                returnString.append("1");
            } else {
                returnString.append("0");
            }
        }
        return returnString;
    }

    static string toIPBinaryString(IPNumeric IPArg) {
        string returnString = "";
        for (int i=3; i>=0; i-=1) {
            returnString += toBinaryString<unsigned char>(IPArg.octets[i]);
            returnString += ".";
        }
        return returnString.erase(returnString.length() - 1);
    }

    friend ostream &operator<<(ostream &stream, IP IPArg) {
        if (binaryFlag) {
            return stream << IPArg.IPBinaryString;
        } else {
            return stream << IPArg.IPString;
        }
    }

    IP operator+(unsigned int operand) {
        return IP(this->IPAddress.IP32 + operand);
    }

    IP operator-(unsigned int operand) {
        return IP(this->IPAddress.IP32 - operand);
    }

    IP operator&(IP operand) {
        return IP(this->IPAddress.IP32 & operand.IPAddress.IP32);
    }

    void operator+=(unsigned int operand) {
        *this = (*this + operand);
    }

    void operator-=(unsigned int operand) {
        *this = (*this - operand);
    }

    void operator&=(unsigned int operand) {
        *this = (*this & operand);
    }

};

class SubnetMask : public IP {
public:
    long blockSize;
    long numberOfSubnets;
    int hostBits;
    int networkBits;

    SubnetMask(string stringArg) : IP(stringArg) {
        if (isCIDRMask(stringArg)) {
            this -> IPAddress.IP32 = ~((1<<(32-stoi(stringArg))) - 1);
            this -> IPString = intToIPString(this -> IPAddress);
            this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
        }
        this -> hostBits = fetchHostBits((int)this -> IPAddress.IP32);
        if (!unusualFormat) {
            this -> blockSize = 1<<this -> hostBits;
        } else {
            this -> blockSize = 0;
        }
        this -> networkBits = 32 - hostBits;
        this -> numberOfSubnets = 1<<networkBits;
    }

    SubnetMask(int CIDRMask) : IP(~((1<<(32-CIDRMask)) - 1)) {
        this -> hostBits = 32 - CIDRMask;
        if (!unusualFormat) {
            this -> blockSize = 1<<this -> hostBits;
            this -> networkBits = CIDRMask;
            this -> numberOfSubnets = 1<<networkBits;
        } else {
            this -> blockSize = 0;
            this -> networkBits = 0;
            this -> numberOfSubnets = 1;
        }
        
    }

    SubnetMask() {}

    static int fetchHostBits(int IPNumber) {
        int IPComplement = (int)(~IPNumber) + 1;
        for (int i=0; i<=32; i++) {
            if (1<<i == IPComplement) {
                return i;
            }
        }
        return 0;
    }

    static bool verifyMask(SubnetMask netArg) {
        if (SubnetMask(netArg.networkBits).IPString.compare(netArg.IPString)) {
            // cout << "Failed test mask " << netArg << " against " << SubnetMask(netArg.networkBits) << endl;
            return false;
        }
        return true;
    }
};

class ChangingIP {
public:
    string IPString;
    string IPBinaryString;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    ChangingIP(IP IPAddress, SubnetMask netMask) {
        this -> IPString = getChangingIPString(IPAddress, netMask);
        this -> IPBinaryString = getChangingIPBinaryString(IPAddress, netMask);
        this -> netMask = netMask;
    }

    ChangingIP() {}

    friend ostream &operator<<(ostream &stream, ChangingIP ChangingIPArg) {
        if (binaryFlag) {
            return stream << ChangingIPArg.IPBinaryString;
        } else {
            return stream << ChangingIPArg.IPString;
        }
    }

    string getChangingIPString(IP IPArg, SubnetMask netMask) {
        string returnString = "";
        int changingOctets = getChangingOctets(netMask);
        for (int i=3; i>=changingOctets; i--) {
            returnString.append(to_string(IPArg.IPAddress.octets[i]));
            returnString.append(".");
        }
        for (int i=0; i<changingOctets; i++) {
            returnString.append("x.");
        }
        returnString.erase(returnString.length() - 1);
        return returnString;
    }

    static string getChangingIPBinaryString(IP IPArg, SubnetMask netMask) {
        string returnString = "";
        int changingBits = netMask.hostBits;
        for (int i=3; i>=0; i--) {
            returnString += IP::toBinaryString(IPArg.IPAddress.octets[i]);
        }
        for (int i=0; i<(int)returnString.length(); i++) {
            if (i>=32-changingBits && returnString[(size_t)i] != '.') {
                returnString[(size_t)i] = 'x';
            }
        }
        for (int i=1; i<=3; i++) {
            returnString.insert(returnString.begin() + 8*i+(i-1), '.'); // new indices each iteration must account for periods being added to string.
        }
        return returnString;
    }

    int getChangingOctets(SubnetMask netMask) {
        return (int)ceil(netMask.hostBits / 8.0); // division requires float argument to return float.
    }
};

class Subnet {
public:

    IP networkIP;
    IP startIP;
    IP endIP;
    IP broadcastIP;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, Subnet subnetArg);

    Subnet(IP IPAddress, SubnetMask netMask) {
        this -> networkIP = IPAddress & netMask;
        this -> startIP = networkIP + 1;
        this -> broadcastIP = networkIP + (unsigned int)(netMask.blockSize - 1);
        this -> endIP = broadcastIP - 1;
        this -> netMask = netMask;
    }

    static string subnetToString(Subnet subnetArg) {
        switch (subnetArg.netMask.blockSize) {
            case 1: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits);
            case 2: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" + subnetArg.networkIP.IPString + " - " + subnetArg.broadcastIP.IPString;
            default: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" + subnetArg.startIP.IPString + " - " 
                    + subnetArg.endIP.IPString + "\n\t" + subnetArg.broadcastIP.IPString + " broadcast";
        }
    }

    static string subnetToBinaryString(Subnet subnetArg) {
        switch (subnetArg.netMask.blockSize) {
            case 1: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits);
            case 2: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" 
                + subnetArg.networkIP.IPBinaryString + " - " + subnetArg.broadcastIP.IPBinaryString;
            default: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" 
                + subnetArg.startIP.IPBinaryString + " - " + subnetArg.endIP.IPBinaryString + "\n\t" + subnetArg.broadcastIP.IPBinaryString 
                + " broadcast";
        }
    }

    friend ostream &operator<<(ostream &stream, Subnet subnetArg) { // change what this does when using binary argument?
        if (binaryFlag) {
            return stream << subnetToBinaryString(subnetArg);
        } else {
            return stream << subnetToString(subnetArg);
        }
    }
};

void VLSM(IP IPAddr, SubnetMask netMask1, SubnetMask netMask2) {
    if (IPAddr.unusualFormat || netMask1.unusualFormat || netMask2.unusualFormat || netMask1.blockSize < netMask2.blockSize) {
        usage();
    }
    IP localIPCopy = IPAddr;
    int networkMagnitudeDifference = netMask1.hostBits - netMask2.hostBits;
    int subnetsToGenerate = 1<<networkMagnitudeDifference;
    if (numberFlag < subnetsToGenerate && numberFlag != 0) {
        subnetsToGenerate = numberFlag;
    }
    cout << subnetsToGenerate << " Subnet(s) Total, " << netMask2.blockSize << " IP(s) Per Subnet" << endl;
    cout << netMask1 << "[/" << netMask1.networkBits << "]";
    if (netMask1.blockSize != netMask2.blockSize) {
        cout << " -> " << netMask2 << "[/" << netMask2.networkBits << "]" << endl << (localIPCopy & netMask1) << "/" << to_string(netMask1.networkBits) << " -> " << ChangingIP(IPAddr, netMask2) << "/" << to_string(netMask2.networkBits) << endl;
    } else {
        cout << endl;
    }
    cout << "-------------------------------------------------------------------" << endl;
    if (reverseFlag) {
        localIPCopy += (unsigned int)netMask2.blockSize * ((unsigned int)subnetsToGenerate - 1);
        for (int i=0; i<subnetsToGenerate; i++) {
            cout << Subnet(localIPCopy, netMask2) << endl;
            localIPCopy -= (unsigned int)netMask2.blockSize;
        }
    } else {
        for (int i=0; i<subnetsToGenerate; i++) {
            cout << Subnet(localIPCopy, netMask2) << endl;
            localIPCopy += (unsigned int)netMask2.blockSize;
        }
    }
}

arguments getArgs(int argc, char **argv) {
    arguments argStruct;
    string currentString;
    smatch matches;
    regex customArgPattern("-\\w+");
    int swapper;
    for (int i=0; i<argc; i++) {
        currentString = string(argv[i]);
        if (IP::isIPFormat(currentString) && !argStruct.IPArgumentFound) {
            argStruct.IPArgumentIndex = i;
            argStruct.IPArgumentFound = true;
        } else if ((!SubnetMask(currentString).IPString.compare(currentString) || IP::isCIDRMask(currentString)) && !argStruct.netMask1Found) {
            argStruct.netMask1ArgumentIndex = i;
            argStruct.netMask1Found = true;
        } else if ((!SubnetMask(currentString).IPString.compare(currentString) || IP::isCIDRMask(currentString)) && !argStruct.netMask2Found) {
            argStruct.netMask2ArgumentIndex = i;
            argStruct.netMask2Found = true;
        } else if (regex_search(currentString, matches, customArgPattern)) {
            argStruct.customArgumentVector = string(argv[i]);
            argStruct.customArgumentFound = true;
        }
    }
    if (!argStruct.netMask1Found || !argStruct.IPArgumentFound) {
        usage();
    }
    if (!argStruct.netMask2Found) {
        argStruct.netMask2ArgumentIndex = argStruct.netMask1ArgumentIndex;
        argStruct.netMask2Found = true;
    }
    SubnetMask testMask1(argv[argStruct.netMask1ArgumentIndex]);
    SubnetMask testMask2(argv[argStruct.netMask2ArgumentIndex]);
    if (!SubnetMask::verifyMask(testMask2) || !SubnetMask::verifyMask(testMask1)) {
        usage();
    }
    if (SubnetMask(argv[argStruct.netMask1ArgumentIndex]).networkBits > SubnetMask(argv[argStruct.netMask2ArgumentIndex]).networkBits) {
        swapper = argStruct.netMask1ArgumentIndex;
        argStruct.netMask1ArgumentIndex = argStruct.netMask2ArgumentIndex;
        argStruct.netMask2ArgumentIndex = swapper;
    }
    return argStruct;
}

void setFlags(arguments args) {
    if (!args.customArgumentVector.compare("")) {
        return;
    }
    smatch matches;
    regex numberExpression("(\\d+)");
    if (regex_search(args.customArgumentVector, matches, numberExpression)) {
        numberFlag = stoi(matches.str(0));
    }
    for (int i=0; i<(int)args.customArgumentVector.length(); i++) {
        switch (args.customArgumentVector[(size_t)i]) {
            case 'b':
                binaryFlag = true;
                break;
            case 'd':
                debugFlag = true;
                break;
            case 'r':
                reverseFlag = true;
                break;
            default:
                continue;
                break;
        }
    }
}

int main(int argc, char **argv) {
    globalArgumentVector = argv;
    globalArgs = getArgs(argc, argv);
    setFlags(globalArgs);
    if (debugFlag) {
        cout << "Debug Information" << endl;
        cout << "\tIP Arg Index: " << globalArgs.IPArgumentIndex << endl;
        cout << "\tSubnet1 Arg Index: " << globalArgs.netMask1ArgumentIndex << endl;
        cout << "\tSubnet2 Arg Index: " << globalArgs.netMask2ArgumentIndex << endl;
        cout << "\tCustom Arg Value: " << globalArgs.customArgumentVector << endl;
        cout << "\tBinary Flag: " << binaryFlag << endl;
        cout << "\tDebug Flag: " << debugFlag << endl;
        cout << "\tReverse Flag: " << reverseFlag << endl;
        cout << "\tNumber Flag: " << numberFlag << endl;
        cout << endl;
    }
    IP myIP(argv[globalArgs.IPArgumentIndex]);
    SubnetMask myMask1(argv[globalArgs.netMask1ArgumentIndex]);
    SubnetMask myMask2(argv[globalArgs.netMask2ArgumentIndex]);
    VLSM(myIP, myMask1, myMask2);
}