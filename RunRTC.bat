@echo This program will run the target RTC

set rtc_name = ImageLoaderComp

@cd .\build\src\release
ImageLoaderComp.exe -f rtc.conf

