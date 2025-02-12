#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <string.h>
#include <ctype.h>

static void uart_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);

    // GPIO_USART1_RX - > GPIO9
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                 GPIO_CNF_INPUT_FLOAT, GPIO_USART1_RX);
    // GPIO_USART1_TX - GPIO10
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                 GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    /* Setup UART parameters. */
    usart_set_baudrate(USART1, 9600);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART1);
}

static void uart_putchar(uint32_t usart, char c)
{
    if (c == '\n')
        usart_send_blocking(usart, '\r');
    usart_send_blocking(usart, c);
}

static void uart_print(uint32_t usart, char *s)
{
    while (*s != 0) {
        uart_putchar(usart, *s);
        s++;
    }
}

static int uart_read(uint32_t usart, char *buf, int len)
{
    char c;
    int i = 0;

    while (i < len) {
        c = usart_recv_blocking(usart);
        if (c == '\r')
            c = '\n';

        uart_putchar(usart, c);
        buf[i++] = c;

        if (c == '\n')
            break;
    }
    return i;
}

#define MAX_PROG 16
static char *prog[MAX_PROG];
static int (*bin[MAX_PROG])(int, char **);

static int exec(int argc, char **argv)
{
    if (argc < 1)
        return -1;

    for (int i = 0; prog[i] != NULL; i++) {
        if (!strcmp(prog[i], argv[0])) {
            bin[i](argc, argv);
            return 0;
        }
    }

    return -1;
}

static int inst(char *_prog, int (*_bin)(int, char **))
{
    int i = 0;
    while (prog[i] != NULL)
        i++;

    if (i >= MAX_PROG)
        return -1;

    prog[i] = _prog;
    bin[i] = _bin;
}

static void delay_1s(void)
{
    int i = 72e6/2/4;

    while (i > 0) {
        i--;
        __asm__("nop");
    }
}

static int help(int argc, char **argv)
{
    for (int i = 0; prog[i] != NULL; i++) {
        uart_print(USART1, prog[i]);
        uart_putchar(USART1, ' ');
    }
    uart_putchar(USART1, '\n');
}

static int blink(int argc, char **argv)
{
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

    // the led lights up when gpio13 is CLEAR instead of SET
    gpio_clear(GPIOC, GPIO13);
    delay_1s();
    gpio_set(GPIOC, GPIO13);

    rcc_periph_clock_disable(RCC_GPIOC);
    return 0;
}

static int sleep(int argc, char **argv)
{
    delay_1s();
    return 0;
}

int main(void)
{
    char buf[512];
    char *argv[32];
    int argc;
    int n;

    //set STM32 to 72 MHz
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

    uart_setup();
    uart_print(USART1, "setting it up...");

    inst("h", help);
    inst("help", help);
    inst("b", blink);
    inst("blink", blink);
    inst("sleep", sleep);

    uart_print(USART1, "done\n");

    while (1) {
        uart_print(USART1, "msh> ");
        n = uart_read(USART1, buf, sizeof(buf));

        for (int i = 0; i < n; i++) {
            if (isspace(buf[i]))
                buf[i] = 0;
        }

        argc = 0;
        for (int i = 0; i < n; i++) {
            if (buf[i]) {
                argv[argc++] = &buf[i];
                while (buf[++i]);
            }
        }

        if (argc == 0)
            continue;

        if (exec(argc, argv) < 0)
            uart_print(USART1, "no such command\n");
    }
}
