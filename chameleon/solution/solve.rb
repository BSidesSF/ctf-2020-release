require 'date'
require 'openssl'

if(!ARGV[1])
  puts("Usage: ruby solve.rb <filename> <keyfile>")
  exit
end
data = File.read(ARGV[0])
sample = data[0..15]

keys = File.read(ARGV[1]).split(/\n/)

keys = ["a051b8a16f8542da"]
keys.each do |k|
  c = OpenSSL::Cipher::DES.new('CBC')
  c.key = [k].pack("H*")
  c.decrypt
  c.padding = 0

  begin
    $stderr.puts("Key: %s" % k)

    out = c.update(sample) + c.final()

    b = out.bytes();
    if(b[0] == 0x89 && b[1] == 0x50 && b[2] == 0x4e && b[3] == 0x47)
      c = OpenSSL::Cipher::DES.new('CBC')
      c.key = [k].pack("H*")
      c.decrypt
      puts c.update(data) + c.final()
      exit(0)
    end
  rescue OpenSSL::Cipher::CipherError
    # Ignore errors, that's gonna happen
  end
end
