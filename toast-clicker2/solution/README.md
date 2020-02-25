# Toast Clicker 2
Flag is encrypted with AES and then base64 encoded. The encrypted flag is placed in `util.cpp` returned by the `encryptedStringFromJNI` function. The key to decrypt it is `MD5(donthidesecretsincode)` = `742375c48a70da605b1649c9b7118e61`, which is split into three parts:
* `742375c48a7` in Build config (`KEY_PART1`)
* `0da605b16` in secrets.xml (`key_part2`)
* `49c9b7118e61` returned by `keyStringFromJNI` function in `util.cpp`

With the key, can decrypt the flag to get `CTF{T00_Many_S3cr3t5}`. Can solve this using Frida / Reversing / Patching APK to log decrypted key to Logcat. 