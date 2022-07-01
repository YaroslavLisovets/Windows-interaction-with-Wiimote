#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!("./bindings.rs");
use crate::leds_states::{LED_1, LED_2, LED_3, LED_4};
use crate::wiimote_states::{ACCELEROMETER_ON, RUMBLE_ON};


pub mod button_states {
    pub const TWO: u16 = 0x0001;
    pub const ONE: u16 = 0x0002;
    pub const B: u16 = 0x0004;
    pub const A: u16 = 0x0008;
    pub const MINUS: u16 = 0x00010;
    pub const HOME: u16 = 0x00080;
    pub const LEFT: u16 = 0x00100;
    pub const RIGHT: u16 = 0x00200;
    pub const DOWN: u16 = 0x00400;
    pub const UP: u16 = 0x00800;
    pub const PLUS: u16 = 0x01000;
}

pub mod wiimote_states {
    pub const ACCELEROMETER_ON: u8 = 0b10000000;
    pub const RUMBLE_ON: u8 = 0b01000000;
}

pub struct Buttons {
    pressed: u16,
    released: u16,
    just_pressed: u16,
}


impl Buttons {
    fn new() -> Buttons {
        Buttons { pressed: 0, released: 0, just_pressed: 0 }
    }

    fn update(&mut self, buf: &[u8]) {
        let mut pressed = 0u16;
        pressed = pressed | (buf[1] as u16);
        pressed = pressed << 8;
        pressed = pressed | (buf[2] as u16);
        self.released = (!pressed) & self.pressed;
        self.just_pressed = (!self.released) & self.pressed ^ pressed;
        self.pressed = pressed;
        // println!("pressed:  {:#016b}", pressed);
        // println!("released: {:#016b}", self.just_pressed);
        for i in &buf[..9] {
            print!("{i:#09b}|");
        }
        print!("\n")
    }
    pub fn is_button_pressed(&self, button: u16) -> bool {
        self.pressed & button != 0
    }
    pub fn is_button_just_pressed(&self, button: u16) -> bool {
        self.just_pressed & button != 0
    }
    pub fn is_button_released(&self, button: u16) -> bool {
        self.released & button != 0
    }
}


pub mod leds_states {
    pub const LED_1: u8 = 0x10;
    pub const LED_2: u8 = 0x20;
    pub const LED_3: u8 = 0x40;
    pub const LED_4: u8 = 0x80;
}

pub struct Wiimote {
    state: u8,
    led_state: u8,
    device: wiimote_hid,
    pub buttons: Buttons,
    accelerometer: [i16; 3],
    accelerometer_initial: [u8; 3],
    buffer: [byte; 22usize],
}

static mut ble_device: Option<*mut bluetooth_device> = None;

impl Wiimote {
    pub fn new() -> Wiimote {
        unsafe {
            let wiimote = GetWiimoteHid();
            Wiimote {
                state: 0,
                led_state: 0,
                device: wiimote,
                buttons: Buttons::new(),
                accelerometer: [0; 3],
                accelerometer_initial: [0; 3],
                buffer: [0; 22usize],
            }
        }
    }

    pub fn find() -> Option<Wiimote> {
        Some(Wiimote::new())
    }

    pub fn bleConnect() {
        unsafe { ble_device = Some(FindConnectWiimoteBLE()); }
    }

    pub fn bleDisconnect() {
        unsafe { wiimoteDisconect(ble_device.unwrap()); }
    }
    //
    pub fn write(&mut self, len: usize) {
        unsafe {
            self.device.buffer = [0u8; 22];
            for n in 0..len + 1 {
                self.device.buffer[n] = self.buffer[n];
            }
            self.device.write();
        }
    }

    //Function will write lat report in buffer
    pub fn sync(&mut self) {
        unsafe {
            let len: usize = self.device.read().try_into().unwrap();
            for n in 0..len {
                self.buffer[n] = self.device.buffer[n];
            }
        }
    }
    pub fn update(&mut self) {
        self.sync();
        self.buttons.update(&self.device.buffer[..]);
        if self.get_state_value(ACCELEROMETER_ON) {}
    }
    pub fn set_vibration(&mut self, working: bool) {
        self.state &= !RUMBLE_ON;
        self.state |= RUMBLE_ON * (working as u8);
        self.buffer[0] = 0x11;
        self.buffer[1] = working as u8;
        self.write(3);
    }
    pub fn set_accelerometer_state(&mut self, working: bool) {
        self.state &= !ACCELEROMETER_ON;
        self.state |= ACCELEROMETER_ON * (working as u8);
        self.buffer = [0; 22];
        self.buffer[0] = 0x12;
        self.buffer[2] = 0x30 | working as u8;
        self.write(3);
    }


    pub fn get_state_value(&self, state: u8) -> bool {
        self.state & state != 0
    }

    pub fn set_state_value(&mut self, state_type: u8, value: bool) {
        self.state |= state_type * (value as u8);
    }

    pub fn set_leds(&mut self, value: u8) {
        self.led_state = value;
        self.buffer[0] = 0x11;
        self.buffer[1] = value;
        self.write(2);
    }

    pub fn set_by_number(&mut self, number: u8) {
        let led = match number {
            1 => { LED_1 }
            2 => { LED_2 }
            3 => { LED_3 }
            _ => LED_4
        };
        self.set_leds(led);
    }
}