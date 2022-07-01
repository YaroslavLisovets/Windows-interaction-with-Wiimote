fn main() {
    println!(r"cargo:rustc-link-search=libs");
    println!("cargo:rustc-link-lib=WiimoteHid");
    println!("cargo:rustc-link-lib=SetupAPI");
    println!("cargo:rustc-link-lib=hid");
    println!("cargo:rustc-link-lib=bluetoothapis");

}