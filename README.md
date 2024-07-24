# MuteMicrophone
Simple C++ application that registers a keyboard shortcut to mute microphone on Windows.

# Why this app exists?
This app was created mainly because Windows for some reason doesn't come with a universal shortcut to mute the microphone. I bought the [Keychron K8 Pro](https://www.keychron.com/products/keychron-k8-pro-qmk-via-wireless-mechanical-keyboard) mechanical keyboard and was very disappointed to find that to use the microphone mute button on the keyboard in Windows, I can only register it to a keyboard shortcut (macro) or to a specific code (the keyboard uses [VIA](https://www.caniusevia.com/)). Since Windows didn't provide either of these for the microphone mute functionality, I decided to create my own solution. It's not that there are no other solutions already, but I wanted to make it as easy and efficient as possible without unnecessary modifications and features and, as a bonus, learn something new. Just emulate the function of the mute button, nothing else.

# Usage
1. Compile the source
2. Open Win+r and type shell:startup
3. Copy the compiled binary to the shell:startup directory

Now the MuteMicrophone app will start automatically every time you start your computer, stay hidden in the background and be available to use right away.

**Additionally**:

If you are using VIA keyboard, you can register a new macro and map it to the *alt-m* keyboard shortcut and use a dedicated button on the keyboard.

# Todo
- [ ] Add indication of microphone being muted