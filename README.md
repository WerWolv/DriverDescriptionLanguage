# Driver Description Language

## What is this?
This Project is an attempt to create a description language to define drivers for hardware peripherals in a reusable manner.
Instead of having to write drivers for the same IC over and over again for different controllers, instead it allows you to simply write one interface peripheral driver for your controller family and then reuse the generic IC driver again without having to rewrite it from scratch.
The compiler will then generate protable C code for you that can be compiled into your project.

## Example

**Simple STM32 I2C driver**
```cpp
namespace STM32 {
  
  driver I2C<u8 Address> : I2C {
    
    // Generic read function
    fn readRegister<T>(u8 register) {
      // Define the result variable
      T result = 0x00;
      
      // Raw C code block to use the STM32 HAL library to read the I2C data
      [[
          HAL_I2C_Master_Transmit(&hi2c1, Address | 0x01, &register, 1, 1000);
          HAL_I2C_Master_Receive(&hi2c1, Address | 0x01, &result, sizeof(result), 1000);
      ]]
      
      // Return the result
      return result;
    }
    
  }
  
}
```

**Simple MAX17261 driver**
```cpp
// Defines a new driver based on the STM32 I2C driver with address 0x6C
driver MAX17261 : {% impl %}::I2C<0x6C> {
  // Function to get the value of the status register
  fn getStatus() => readRegister<u16>(0x00);
}
```

To use these drivers now, you need to write a specs file that tells the compiler how to combine these files into C code.

```toml
# MAX17261 Fuel Gauge
[MAX17261]
path = "max17261.drv"                     # Load its definition from the max17261.drv file
config = { impl = "STM32" }               # Set the placeholders, define "impl" as "STM32"
depends = ["STM32"]                       # Define the dependencies

# STM32 I2C driver
[STM32]
path = "stm32.drv"                        # Load its definition from the stm32.drv file
```
