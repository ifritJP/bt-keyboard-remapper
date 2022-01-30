HOSTPC=
ESP32_IP=
OTA_AUTH=


ifneq ($(wildcard private/make.conf.mk),)
include private/make.conf.mk
endif

COM=rfc2217://$(HOSTPC):5555
RATE=1500000

IDF_OPT=-b $(RATE) -p $(COM)

PROJECT_NAME=$(shell grep -e '^project' CMakeLists.txt | sed 's/.*(\(.*\))/\1/g')


.PHONY: build

help:
	@echo make all
	@echo make build
	@echo make burn
	@echo make monitor
	@echo make ota

all:
	idf.py flash $(IDF_OPT) monitor 


build:
	idf.py app

burn:
	idf.py flash $(IDF_OPT)

monitor:
	idf.py monitor -p $(COM) 

only-ota:
ifndef ESP32_IP
	@echo "\nno IP_ADDR."
else
	cat build/$(PROJECT_NAME).bin | \
		curl http://$(ESP32_IP)/ota/ --no-buffer \
			--user "$(OTA_AUTH)" --data-binary @-
endif

ota: build
	$(MAKE) only-ota

make_parttable:
	python ${IDF_PATH}/components/partition_table/gen_esp32part.py \
		--verify partitions.csv binary_partitions.bin

flash_parttable:
	idf.py partition-table-flash
