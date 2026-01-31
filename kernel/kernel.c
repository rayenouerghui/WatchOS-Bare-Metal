// kernel.c
// This is our first C code running in kernel mode

void kernel_main() {
    // VGA text mode buffer starts at address 0xB8000
    volatile char* video_memory = (volatile char*) 0xB8000;

    // Text we want to display
    const char* message = "Kernel started successfully";

    // Color: light grey (0x0F) on black background
    char color = 0x0F;

    int i = 0;

    // Write each character to video memory
    while (message[i] != '\0') {
        video_memory[i * 2] = message[i];      // Character
        video_memory[i * 2 + 1] = color;       // Color attribute
        i++;
    }

    // Stop here forever
    while (1) {}
}

