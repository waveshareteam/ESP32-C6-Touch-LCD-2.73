/* SPDX-License-Identifier: Apache-2.0 */
/* CH32V006: keep the 2.73 register protocol and 16-bit output mask. */
#include "io_extension.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static const char *TAG = "ch32v006";
static i2c_master_dev_handle_t device;
static StaticSemaphore_t lock_storage;
static SemaphoreHandle_t lock;
static uint16_t outputs = 0xFFFF;
#define TIMEOUT_MS 100
esp_err_t IO_EXTENSION_Init(i2c_port_num_t port)
{
    if (device) return ESP_OK;
    if (!lock) lock = xSemaphoreCreateMutexStatic(&lock_storage);
    if (!lock) return ESP_ERR_NO_MEM;
    i2c_master_bus_handle_t bus;
    ESP_RETURN_ON_ERROR(i2c_master_get_bus_handle(port, &bus), TAG, "I2C bus");
    const i2c_device_config_t config = {
        .dev_addr_length=I2C_ADDR_BIT_LEN_7, .device_address=IO_EXTENSION_ADDR, .scl_speed_hz=100000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus,&config,&device),TAG,"CH32V006");
    const uint8_t mode[]={IO_EXTENSION_Mode,0xF7,0xFF};
    esp_err_t ret=i2c_master_transmit(device,mode,sizeof(mode),TIMEOUT_MS);
    if(ret!=ESP_OK) { i2c_master_bus_rm_device(device); device=NULL; }
    return ret;
}
static esp_err_t read_bytes(uint8_t reg,uint8_t *bytes,size_t length)
{
    if(!device) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(lock,portMAX_DELAY);
    esp_err_t ret=i2c_master_transmit_receive(device,&reg,1,bytes,length,TIMEOUT_MS);
    xSemaphoreGive(lock); return ret;
}
esp_err_t IO_EXTENSION_Write_Output(uint8_t pin,uint8_t value)
{
    if(pin>15 || pin==3) return ESP_ERR_INVALID_ARG;
    if(!device) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(lock,portMAX_DELAY);
    uint16_t next=value ? outputs|(1u<<pin) : outputs&~(1u<<pin);
    uint8_t bytes[]={IO_EXTENSION_IO_OUTPUT_ADDR,next&0xFF,next>>8};
    esp_err_t ret=i2c_master_transmit(device,bytes,sizeof(bytes),TIMEOUT_MS);
    if(ret==ESP_OK) outputs=next;
    xSemaphoreGive(lock); return ret;
}
esp_err_t IO_EXTENSION_Write_PWM(uint8_t value)
{
    if(!device) return ESP_ERR_INVALID_STATE;
    /* Match the original CH32V006 PWM range, including its 97% upper limit. */
    if(value>97) value=97;
    uint8_t bytes[]={IO_EXTENSION_PWM_ADDR,(uint8_t)(value*255u/100u)};
    xSemaphoreTake(lock,portMAX_DELAY);
    esp_err_t ret=i2c_master_transmit(device,bytes,sizeof(bytes),TIMEOUT_MS);
    xSemaphoreGive(lock); return ret;
}
esp_err_t IO_EXTENSION_Read_Input(uint8_t pin,uint8_t *value)
{
    if(pin>15 || !value) return ESP_ERR_INVALID_ARG;
    uint8_t bytes[2];
    esp_err_t ret=read_bytes(IO_EXTENSION_IO_INPUT_ADDR,bytes,sizeof(bytes));
    if(ret==ESP_OK) *value=(((uint16_t)bytes[1]<<8)|bytes[0])>>pin&1;
    return ret;
}
esp_err_t IO_EXTENSION_Read_ADC(uint16_t *value)
{
    if(!value) return ESP_ERR_INVALID_ARG;
    uint8_t bytes[2];
    esp_err_t ret=read_bytes(IO_EXTENSION_ADC_ADDR,bytes,sizeof(bytes));
    if(ret==ESP_OK) *value=((uint16_t)bytes[1]<<8)|bytes[0];
    return ret;
}
void IO_EXTENSION_IO_Mode(uint16_t mask)
{
    if(!device) return;
    /* EXIO3 is the charging input; never turn it into an output. */
    mask &= ~(1u<<3);
    uint8_t bytes[]={IO_EXTENSION_Mode,mask&0xFF,mask>>8};
    xSemaphoreTake(lock,portMAX_DELAY);
    esp_err_t ret=i2c_master_transmit(device,bytes,sizeof(bytes),TIMEOUT_MS);
    xSemaphoreGive(lock);
    if(ret!=ESP_OK) ESP_LOGW(TAG,"Direction write: %s",esp_err_to_name(ret));
}
void IO_EXTENSION_Output(uint8_t pin,uint8_t value) { (void)IO_EXTENSION_Write_Output(pin,value); }
void IO_EXTENSION_Pwm_Output(uint8_t value) { (void)IO_EXTENSION_Write_PWM(value); }
uint8_t IO_EXTENSION_Input(uint8_t pin) { uint8_t v=0; (void)IO_EXTENSION_Read_Input(pin,&v); return v; }
uint16_t IO_EXTENSION_Adc_Input(void) { uint16_t v=0; (void)IO_EXTENSION_Read_ADC(&v); return v; }
uint8_t IO_EXTENSION_RTC_INT_READ(void) { uint8_t v[2]={0}; (void)read_bytes(IO_EXTENSION_RTC_INT_ADDR,v,2); return v[0]; }
