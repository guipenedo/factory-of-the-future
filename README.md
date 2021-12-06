# factory-of-the-future

## Setting up on a raspberry
SSH into the raspberry and run the following:
```bash
git clone https://github.com/guipenedo/factory-of-the-future.git
cd factory-of-the-future
sudo apt-get update
sudo apt-get install -y cmake gnuplot-x11 libcanberra-gtk-module libcanberra-gtk3-module
cmake .
```

These commands will download the code, install and run cmake (which will then create our makefile).

## Applying updates
When there is an update to the repository code, you can simply run the following commands:
```bash
git pull
cmake .
```

## Running the dashboard
Before running the dashboard make sure you find its ipv4 IP address by running `ifconfig`.
Afterwards, simply do:
```bash
make dashboard
./dashboard
```

## Running a factory
You will need the dashboard IP to run a factory (the dashboard should be started before the factories):
```bash
make factory
./factory [dashboard ip address]
```

## Running the ML Module
You will need the dashboard IP to run a factory (the dashboard should be started before the factories):
```bash
make ml
./ml [dashboard ip address]
```