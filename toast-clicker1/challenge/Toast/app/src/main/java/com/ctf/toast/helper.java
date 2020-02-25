package com.ctf.toast;
import javax.crypto.Cipher;
import android.util.Log;
import java.security.MessageDigest;
import java.util.Arrays;
import android.util.Base64;
import javax.crypto.spec.SecretKeySpec;

public class helper {
    private SecretKeySpec key;
    private Cipher cipher;
    private String passphrase;
    public helper(String passphrase) throws Exception{
        this.passphrase = passphrase;
        key = initKey();
        cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
    }

    public SecretKeySpec initKey() throws Exception{
        byte[] keyBytes = null;
        MessageDigest md;
        SecretKeySpec keySpec;
            keyBytes = passphrase.getBytes("UTF-8");
            md = MessageDigest.getInstance("SHA-1");
            keyBytes = md.digest(keyBytes);
            keyBytes = Arrays.copyOf(keyBytes, 16);
            keySpec = new SecretKeySpec(keyBytes, "AES");
        return keySpec;
    }

    public String encrypt (String plaintext) throws Exception{
        byte[] plaintextBytes = plaintext.getBytes();
        cipher.init(Cipher.ENCRYPT_MODE, key);
        byte[] ciphertext =  cipher.doFinal(plaintextBytes);
        return Base64.encodeToString(ciphertext,Base64.NO_WRAP);
    }

    public String decrypt (String ciphertext) throws Exception{
        byte[] ciphertextBytes = Base64.decode(ciphertext.getBytes(),Base64.NO_WRAP);
        cipher.init(Cipher.DECRYPT_MODE, key);
        byte[] plaintext = cipher.doFinal(ciphertextBytes);
        return new String (plaintext, "UTF-8");
    }
}


