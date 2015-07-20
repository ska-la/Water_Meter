# Water_Meter
A water meter for FBU (Federal Budget Unit) Rostov CSM.

Hardware framework consists from Arduino board like Pro Mini (ATmega328 uC), LCD 16x2 (or 20x4), keypad 4x4 (or something resemble). Two input signals are processed. First signal comes from a water flow meter (litre per pulse), second one comes from an external start/stop button, which marks an interval of measuring.
Software is developed with Arduino IDE. It includes a [LiquidCrytal_i2c library's patch](https://github.com/ska-la/arduino_patches/blob/master/LiquidCrystal_I2C.patch) and [kPad_i2c library](https://github.com/ska-la/kPad_i2c).
