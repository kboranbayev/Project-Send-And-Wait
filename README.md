# Project-Send-And-Wait
The objective of this project is to design and implement a basic Send-And-Wait protocol simulator. The protocol will be half-duplex and use sliding windows to send multiple packets between to hosts on a LAN with an “unreliable network” between the two hosts.

## Install

### Configure

Edit config.txt.

First line will represent the server IP address and Port number
Second line will represent the emulator IP address and Port number

Run BASH script
```
sh compile.sh
```
The script will compile and create three executables:

```
server, emulator, client
```

### Run

Note: Run each of the scripts in each terminal in following order:

```
./server
```

```
./emulator -n 0 -d 0 -f config.txt
```

```
./client -f data/test.txt [emulator ip address] [emulator port]
```
## Authors

* **Kuanysh Boranbayev** - *Initial work* - [kboranbayev](https://github.com/kboranbayev)
* **Parm Dhaliwal** - *Initial work* - [ParmDhaliwal](https://github.com/ParmDhaliwal)

## Acknowledgments

* Aman Abdulla