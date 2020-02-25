# Toast Clicker 3
When you `Upgrade to Pro` you send a request to pull the `.dex` file from `https://storage.googleapis.com/bsides-sf-ctf-2020-attachments/bacon-final.dex`, and dynamically loads it. It uses a Bacon cipher, which is a substitution cipher. It has three flag parts that it combines and decodes in `bacon.ToastDynamicFlag` to produce `CTF{makingbaconpancakes}`.

Can be solved by pulling the `bacon-final.dex` and reversing it using `dex2jar` or Frida or patching the APK to log the flag.