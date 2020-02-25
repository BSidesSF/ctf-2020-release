file = ARGV[0]
if !file
  $stderr.puts "Expected: ruby solve.rb <filename.png>"
  exit 1
end

if ! system "convert -crop 538x144+138+1111 #{file} ./temp_image.txt"
  $stderr.puts "Failed to convert file to txt"
  exit 1
end

data = File.read("./temp_image.txt").split("\n")

pixels = []
biggest_x = 0
data.each do |d|
  d.scan(/^([0-9]+),([0-9]+):.* ([a-z]+)$/).each do |parsed|
    # Break out the fields
    x = parsed[0].to_i
    #y = parsed[1].to_i
    color = parsed[2]

    # Save the biggest so we can find the "right" part of the pixels
    if(x > biggest_x)
      biggest_x = x + 1
    end

    # If it was black or white, save the pixel
    if(color == 'black' || color == 'white')
      if(pixels[x] && pixels[x] != color)
        $stderr.puts("Uh oh! We have a conflict at x = #{x}!")
        exit(1)
      end

      #$stderr.puts("pixels[#{x}] => #{color}")
      pixels[x] = color
    end
  end
end

if(pixels.length != biggest_x)# || pixels.any?(nil))
  $stderr.puts("Unsolveable! #{pixels.length} :: #{biggest_x}")
  exit(1)
else
  # Convert it to an imagemagick text file
  out = []
  out << "# ImageMagick pixel enumeration: 538,144,65535,srgba"
  0.upto(143) do |y|
    pixels.each_with_index do |p, i|
      p = (p == 'black' ? ': (0,0,0,65535)  #000000FF  black' : ': (65535,65535,65535,65535)  #FFFFFFFF  white')
      out << "#{i},#{y}#{p}"
    end
  end

  # Write out the temp image
  File.write('./solution.txt', out.join("\n"))

  # Convert
  system('convert solution.txt solution.png')
  puts('Solution written to solution.png')
end
