# About

This is an educational project to refresh and learn some C skills in

* SDL graphics programming
* pthread handling
* curl and json parsing
* ini-style config reading
* dlopen pluging handling

Currently it only queries the OpenWeatherMap API and displays the current weather
on screen via SDL, which - with proper SDL build - also works X-less on a RasPi.

# Build and Run

* Obtain an API key at https://openweathermap.org/current and place it in etc/openweathermap.key

```
make && ./westation $(cat etc/openweathermap.key | tr -d "\n")
```

# Random Notes

## Install SDL

SDL must be installed manually on Raspi, otherwise it doesn't work without X.

* Download from https://www.libsdl.org/download-2.0.php
* tar zxvf SDL2-2.0.3.tar.gz
* cd SDL2-2.0.3 && mkdir build && cd build
* ../configure --host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl
* make -j4 && make install

Download and install SDL_image and SDL_ttf (without configure flags).

### Note for SDL_ttf:
* fails to link in the end due to missing libGL.so, just created manual link
** cd /usr/lib/arm-linux-gnueabihf/
** sudo ln -s libGL.so.1 libGL.so

In general, disable full OpenGL, otherwise SDL fails to initialize:
* sudo raspi-config
* Advanced -> GL Driver -> GL (Fake KMS)

## Resources

### Install LCD shield
* https://www.waveshare.com/wiki/3.5inch_RPi_LCD_(A)

### Compile SDL for RasPi non-X
* https://solarianprogrammer.com/2015/01/22/raspberry-pi-raspbian-getting-started-sdl-2/

### Weather Icons
* http://www.alessioatzeni.com/meteocons/

### Batch-Convert SVG to PNG on Mac with proper transparency
* rsvg-convert -h 160 SVG/5.svg > PNG/5.png
** via ```brew install librsvg```

### Remove SVG elemement and convert to PNG
* westation/assets/meteocons-icons/svg2png.sh

### Query openweathermap API
```
KEY=$(cat etc/openweathermap.key | tr -d "\n");
curl "https://api.openweathermap.org/data/2.5/weather?APPID=$KEY&zip=1100,AT&units=metric&lang=de";
echo
```
* https://openweathermap.org/weather-conditions
