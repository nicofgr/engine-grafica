

## Dependencies

### SDL2
```bash
sudo apt-get install libsdl2-2.0-0
sudo apt-get install libsdl2-dev
```

### FreeType
```bash
sudo apt install libfreetype6-dev
```

### All deps
```bash
sudo apt install -y libsdl2-2.0-0 libsdl2-dev libfreetype6-dev
```

### GameNetworkingSockets

mkdir build
cd build
cmake -G Ninja ..
ninja

cd ..
sudo cmake --install build
