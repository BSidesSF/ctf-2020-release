require 'json'
require 'openssl'
require 'securerandom'
require 'sinatra'

set :bind, ENV['HOST'] || '0.0.0.0'
set :port, ENV['PORT'] || '4567'

data = '{}'
begin
  data = File.read('./data.json')
rescue Errno::ENOENT
  # Ignore, just use the default
end

# Note: Decided to do away with transport-level encryption for simplicity (for
# the players, not for me, since I already wrote it :) )
## Binary in, binary out
#def encrypt(d)
#  c = OpenSSL::Cipher::AES.new('CBC')
#  c.encrypt
#  c.key = KEY
#
#  return (c.update(d.to_json) + c.final()).unpack('H*')
#end
#
#def decrypt(e)
#  c = OpenSSL::Cipher::AES.new('CBC')
#  c.decrypt
#  c.key = KEY
#
#  return c.update(d) + c.final()
#end

DATABASE = JSON.parse(data)

not_found do
  return 200, '<p>ERROR: This is an API server, POST to /api/store and /api/retrieve</p>'
end

post '/api/store' do
  key = request.body.read

  if(key.nil?)
    return 400, 'Missing key (should be in POST data)'
  end

  if(key !~ /^[a-f0-9]{16}$/)
    return 400, 'Invalid key - must be 16 hex characters (8 bytes)'
  end
  id = SecureRandom.uuid

  DATABASE[id] = { 'id' => id, 'key' => key }
  File.write('./data.json', DATABASE.to_json())

  puts("Saved key: #{id} => #{key}")

  return 200, id
end

post '/api/retrieve' do
  id = request.body.read

  if(id.nil?)
    return 400, 'Missing identifier (should be in POST data)'
  end

  if(id !~ /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/)
    return 400, 'Bad identifier - must be a UUID'
  end

  entry = DATABASE[id]
  if(entry.nil?)
    return 400, 'ID not found'
  end

  return 200, entry['key']
end
