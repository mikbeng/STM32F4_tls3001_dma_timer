# STM32F4_tls3001_dma_timer

This needs further work. What I got working so far:
*  Working PWM on TIM3 with DMA writes from memory to peripheral register TIM3->CCMR1, bits OC1M. These control the PWM mode and if I change the PWM mode at every TIM update we can generate a Manchester waveform as needed. 

What does not work:
*  Need some FIFO buffer for the DMA transfers. The DMA buffer right now is like 
```uint16_t data_in_buffer [10] = {1, 1, 0, 0, 1, 1, 1, 0, 0, 0};```
Where each element represents a manchester bit. This will be alot of data when running on 200+pixels RGB. A more efficient way would be to have the data on the form 0x0338. A way to implement this could be to continously feed the ``user_init_DMA_buffer`` function at DMA transfer complete interrupt.  


Experimental code. Not really working.. See latest working repository with ESP32 and SPI: https://github.com/mikbeng/esp32_TLS3001_spi.git
