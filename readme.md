# PiscOS
Simple operating system for learning

## Features
- Multiprocess
- Directories
- Shell

## Running

### Dependencies
```
sudo apt install nasm bcc bin86
```

### Compiling
```
./compile.sh
```

### Running
Use Bochs
```
cd build
bochs -f opsys.bxrc
```