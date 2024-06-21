#include <msp430.h>

#define SEG_A BIT1
#define SEG_B BIT2
#define SEG_C BIT3
#define SEG_D BIT4
#define SEG_E BIT5
#define SEG_F BIT6
#define SEG_G BIT7

#define DIGIT1 BIT2 // 十位
#define DIGIT2 BIT0 // 個位

#define LED1 BIT0
#define LED2 BIT1
#define LED3 BIT2
#define LED4 BIT4

int tempC;
float aaaa;


void led_init(void) {
    P2DIR |= (LED1 | LED2 | LED3 | LED4);
    P2OUT &= ~(LED1 | LED2 | LED3 | LED4);
}

void led_update(void) {
    P2OUT &= ~(LED1 | LED2 | LED3 | LED4);      //off leds
    if (tempC < 30) {
        P2OUT |= (LED1);
    } else if (tempC <= 50) {
        P2OUT |= (LED1 | LED2);
    } else if (tempC <=70) {
        P2OUT |= (LED1 | LED2 | LED3);
    } else {
        P2OUT |= (LED1 | LED2 | LED3 | LED4);
    }
}

void digit_init(void) {
    P1DIR |= (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G); // 设置七段显示器的引脚为输出
    P3DIR |= (DIGIT1 | DIGIT2); // 设置位选择引脚为输出
}

void display_digit(int digit) {
    P1OUT |= (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G); // 将所有段关闭 (设为高电平)
    switch(digit) {
        case 0:
            P1OUT &= ~(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);break;
        case 1:
            P1OUT &= ~(SEG_B | SEG_C);break;
        case 2:
            P1OUT &= ~(SEG_A | SEG_B | SEG_G | SEG_E | SEG_D);break;
        case 3:
            P1OUT &= ~(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G);break;
        case 4:
            P1OUT &= ~(SEG_B | SEG_C | SEG_F | SEG_G);break;
        case 5:
            P1OUT &= ~(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G);break;
        case 6:
            P1OUT &= ~(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);break;
        case 7:
            P1OUT &= ~(SEG_A | SEG_B | SEG_C);break;
        case 8:
            P1OUT &= ~(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);break;
        case 9:
            P1OUT &= ~(SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G);break;
        default:
            P1OUT |= (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G); // 清除所有段
            break;
    }
}

void setup_adc(void) {
    // 配置 P5.0 为 ADC 输入引脚
    P5SEL0 |= BIT0;
    P5SEL1 |= BIT0;

    // ADC 配置
    ADCCTL0 = ADCSHT_3 | ADCON; //32 个 ADC 时钟周期
    ADCCTL1 = ADCSSEL_1 | ADCSHP; // 这里以ACLK为例，可能需要根据实际情况调整
    ADCCTL2 = ADCRES_2; // 12位分辨率
    ADCMCTL0 |= ADCSREF_0 | ADCINCH_8; // 选择 A8 输入通道 avcc3.3v
}

int get_temp(void) {
    // 启用 ADC 和开始转换
    ADCCTL0 |= ADCENC | ADCSC;

    // 等待转换完成
    while ((ADCIFG & ADCIFG0) == 0);

    // 读取 ADC 转换结果并计算温度3.3v/4096*100
    aaaa = ADCMEM0;
    return ADCMEM0 * 0.080586;
}

void setup_timer(void) {
    TB0CCR0 = (32768 - 1)/2; // 0.5秒周期 (假设ACLK=32768Hz)
    TB0CCTL0 = CCIE;     // 使能TB0CCR0中断
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR; // ACLK, up mode, clear TBR
}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B(void) {

    tempC = get_temp();
    led_update();
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // 停用看门狗定时器
    PM5CTL0 &= ~LOCKLPM5;       // 解除 GPIO 上的锁定
    digit_init();               // 设置七段显示器
    led_init();                 // 初始化 LED
    setup_adc();                // 配置 ADC
    setup_timer();              // 配置定时器

    __bis_SR_register(GIE);     // 使能全局中断

    while (1) {
        // 更新七段显示器
        int digit1 = tempC / 10; // 十位数
        int digit2 = tempC % 10; // 个位数

        // 切换位选择引脚以选择显示的位
        P3OUT |= (DIGIT1 | DIGIT2); // 两个位都关闭
        P3OUT &= ~DIGIT2; // 打开第一位
        display_digit(digit1); // 显示十位数
        __delay_cycles(10000); // 延迟一段时间以确保显示

        P3OUT |= (DIGIT1 | DIGIT2); // 两个位都关闭
        P3OUT &= ~DIGIT1; // 打开第二位
        display_digit(digit2); // 显示个位数
        __delay_cycles(10000); // 延迟一段时间以确保显示
    }
}
