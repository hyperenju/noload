# noload
![Image](https://github.com/user-attachments/assets/4de509d5-371f-470c-8589-c35e70d3edd2)

# Get Started
```bash
make # build
sudo insmod noload.ko load=10000 # install
sudo rmmod noload # uninstall
```

# What it does
- Creates D-state kernel threads to demonstrate high load average, with near-zero CPU usage.

# Notes
- Just a joke kernel module. Don't use in production.
