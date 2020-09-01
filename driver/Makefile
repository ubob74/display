st-$(CONFIG_ILITEK) += st_display_ilitek.o
st-$(CONFIG_ST7735R) += st_display_st7735r.o
st-y += st_driver.o

obj-m += st.o

all:
	make -C $(KSRC) M=$(PWD) modules
clean:
	make -C $(KSRC) M=$(PWD) clean
	rm -f *.o *~

