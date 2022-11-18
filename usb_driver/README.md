## usb host and usb gadget user guide

## usb host

usb host should run on another linux platform which has a usb 2.0 or usb 3.0 interface;

## usb gadget

I used a raspberry-Pi-4 type-C as a usb gadget, and the gadget need connect to usb host with a type-C line, then need to do as follow:

- make `usb.sh` executable with `chmod +x usb.sh`
- Add `usb.sh` to `/etc/rc.local` before `exit 0` (I really should add a systemd startup script here at some point)