# Subnetter++

## Overview

This program is meant to automatically generate a list of subnets using an IP address and a VLSM range generated from one or two subnet masks.

Each subnet will contain a network address, first usable address, final usable address, and broadcast address unless they are `/31` or `/32` subnets.

`/31` subnets will only have a first usable and final usable address; network and broadcast addresses are meaningless for IP address blocks of size 2 or fewer.

`/32` subnets only have one address, and as such, only one address will be displayed.

## Features

- Generating a VLSM range (Ex: 4 subnets when using a /24 address block to generate /26 subnets).
- Option to present IP addresses in binary format.
- Option to show subnets in reverse block order.
- Option to show debug information.
- Option to limit number of subnets generated.

## Usage

Subnetter++ needs an IP address and subnet mask at minimum to run:

`subnet ipAddr netmask1 <netmask2>`

The netmasks can be passed in full subnet mask or CIDR mask format.

Without a second netmask argument, Subnetter++ will only generate a single subnet with the subnet mask `netmask1`.

If `netmask2` is provided, the number of subnets generated will be equal to two raised to the power of `netmask2`'s host bits - `netmask1`'s host bits.

- Ex: If `netmask1` is 255.255.255.0(/24) and `netmask2` is 255.255.255.128(/25), the number of subnets generated is 2 ^ (25 - 24) => `2`.

- Furthermore, the order the netmasks are provided does not matter; `subnet 172.16.1.1 255.255.255.0 255.255.255.128` will generate the same output as `subnet 172.16.1.1 255.255.255.128 255.255.255.0`.

- However, to avoid ambiguity, the reference IP address must be the first IP-formatted argument.

If the same netmask is given twice, the duplicate netmask is omitted and the program will run as if it were passed only one subnet mask as an argument.

### Optional Flags
---
All optional flags are passed in under a single argument beginning with a `-`.
- This argument does not have to be in a specific location, and the flags do not have to be in a specific order.

The letter `b` represents the binary flag. This flag causes all IP addresses to be shown in 32-bit binary form.

The letter `r` represents the reverse flag. This flag forces subnets to be generated in reverse. If IP addresses can be imagined as 32-bit numbers, this flag makes the numbers shown descend from largest to smallest. The default is to ascend from smallest to largest.

The letter `d` represents the debug flag. It enables the showing of debug information.

The first number that can be grabbed from the optional flag argument is used as the subnet limiter flag. The subnet limiter flag restricts the maximum number of subnets that can be generated.

For example, the following command will only generate ten subnets in binary format, though it would generate 16 otherwise:

`subnet -b10 10.0.0.1 20 24`


## Output Examples

Passing in 192.168.1.1 and 255.255.255.0 as arguments gives the output:
```
$ subnet 192.168.1.1 255.255.255.0
1 Subnet(s) Total, 256 IP(s) Per Subnet
255.255.255.0[/24]
-------------------------------------------------------------------
192.168.1.0/24
        192.168.1.1 - 192.168.1.254
        192.168.1.255 broadcast
```

CIDR masks can also be used interchangeably with full subnet masks; the command below outputs the same as the above:

`subnet 192.168.1.1 24`

If the resulting subnets are of mask /31, network and broadcast addresses are not formally shown:
```
$ subnet 192.168.1.1 30 31
2 Subnet(s) Total, 2 IP(s) Per Subnet
255.255.255.252[/30] -> 255.255.255.254[/31]
192.168.1.0/30 -> 192.168.1.x/31
-------------------------------------------------------------------
192.168.1.0/31
        192.168.1.0 - 192.168.1.1
192.168.1.2/31
        192.168.1.2 - 192.168.1.3
```

If the resulting subnets are of mask /32, the address range is also omitted:
```
$ subnet 192.168.1.1 31 32
2 Subnet(s) Total, 1 IP(s) Per Subnet
255.255.255.254[/31] -> 255.255.255.255[/32]
192.168.1.0/31 -> 192.168.1.1/32
-------------------------------------------------------------------
192.168.1.1/32
192.168.1.2/32
```

Passing in two subnet masks gives the output:
```
$ subnet 192.168.1.1 255.255.255.0 255.255.255.128
2 Subnet(s) Total, 128 IP(s) Per Subnet
255.255.255.0[/24] -> 255.255.255.128[/25]
192.168.1.0/24 -> 192.168.1.x/25
-------------------------------------------------------------------
192.168.1.0/25
        192.168.1.1 - 192.168.1.126
        192.168.1.127 broadcast
192.168.1.128/25
        192.168.1.129 - 192.168.1.254
        192.168.1.255 broadcast
```

The command using the reversed flag shows:
```
$ subnet -r 192.168.1.1 255.255.255.0 255.255.255.128
2 Subnet(s) Total, 128 IP(s) Per Subnet
255.255.255.0[/24] -> 255.255.255.128[/25]
192.168.1.0/24 -> 192.168.1.x/25
-------------------------------------------------------------------
192.168.1.128/25
        192.168.1.129 - 192.168.1.254
        192.168.1.255 broadcast
192.168.1.0/25
        192.168.1.1 - 192.168.1.126
        192.168.1.127 broadcast
```

The limiting the above command's number of subnets generated to 1 outputs:
```
$ subnet -r1 192.168.1.1 255.255.255.0 255.255.255.128
1 Subnet(s) Total, 128 IP(s) Per Subnet
255.255.255.0[/24] -> 255.255.255.128[/25]
192.168.1.0/24 -> 192.168.1.x/25
-------------------------------------------------------------------
192.168.1.0/25
        192.168.1.1 - 192.168.1.126
        192.168.1.127 broadcast
```

Finally, the `-rb1` flag can be passed to give the output in binary:
```
$ subnet -rb1 192.168.1.1 255.255.255.0 255.255.255.128
1 Subnet(s) Total, 128 IP(s) Per Subnet
11111111.11111111.11111111.00000000[/24] -> 11111111.11111111.11111111.10000000[/25]
11000000.10101000.00000001.00000000/24 -> 11000000.10101000.00000001.0xxxxxxx/25
-------------------------------------------------------------------
11000000.10101000.00000001.00000000/25
        11000000.10101000.00000001.00000001 - 11000000.10101000.00000001.01111110
        11000000.10101000.00000001.01111111 broadcast
```
