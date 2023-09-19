#![no_std]
#![no_main]

extern {
    fn printf(p: *const u8);
}

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

fn main() {
    unsafe {
        printf("str".as_ptr() as *const u8);
    }
    loop {}
}
