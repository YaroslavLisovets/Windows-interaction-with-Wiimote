use wiimotelib;
use wiimotelib::{button_states, wiimote_states, Buttons, Wiimote};
use wiimotelib::button_states::{A, B, MINUS, ONE, TWO, UP};
use wiimotelib::leds_states::{LED_1, LED_2, LED_3, LED_4};
use wiimotelib::wiimote_states::{ACCELEROMETER_ON, RUMBLE_ON};

fn main() {
    println!("Нажмите кнопки 1 и 2 одновременно или кнопку синхронизации в отсеке с аккумуляторами.");
    Wiimote::bleConnect();
    let message = "Нажимайте кнопки 1 или 2 для изменения состояния светодиодного индикатора\n\
    Нажмите B для включения/отключения вибрации\n\
    Нажмите A, чтобы включить/отключить акселерометр\n\
    Нажмите вверх на DPAD'е, чтобы получить информацию о разработчике и приложении\n\
    Одновременно нажмите A и Минус, чтобы отключить геймпад";
    println!("Геймпад подключён");
    println!("{message}");
    let mut wiimote = Wiimote::find().unwrap();
    let mut vibration:bool=false;
    let mut led_number = 1u8;
    loop {
        wiimote.update();
        if wiimote.buttons.is_button_just_pressed(B) {
            vibration=!vibration;
            wiimote.set_vibration(!wiimote.get_state_value(RUMBLE_ON));
        }
        if wiimote.buttons.is_button_just_pressed(A){
            wiimote.set_accelerometer_state(!wiimote.get_state_value(ACCELEROMETER_ON));
        }
        if wiimote.buttons.is_button_just_pressed(ONE){
            led_number += 1;
            if led_number == 4{ led_number = 0;}
            wiimote.set_by_number(led_number);
        }
        if wiimote.buttons.is_button_just_pressed(TWO){
            led_number -= 1;
            if led_number == 0{ led_number = 4;}
            wiimote.set_by_number(led_number);
        }
        if wiimote.buttons.is_button_pressed(A)&&wiimote.buttons.is_button_pressed(MINUS){
            Wiimote::bleDisconnect();
            break;
        }
        if wiimote.buttons.is_button_pressed(UP){
            println!("Данное приложение было разработано Лисовцом Я.В. Оно  позволяет взаимодействовать с геймпадом Wiimote.")
        }
    }
}

