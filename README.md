Fool a Kindle to start up without a battery. The Kindle uses SMbus, similar to I2C to read the battery voltage. If your battery is dead you can't run the Kindle DX from a power supply. The trick is to hack it with this and then once it is up you can get into the OS and disable the battery health check.

This PIC program writes a full voltage value to the SMbus
