mult = [1.0, 9.0/8.0, 81.0/64.0, 3.0/2.0, 27.0/16.0]
escala = []
[1,2,4,8].each do |oct|
  mult.each do |m|
    escala << oct.to_f * m.to_f * 220.0
  end
end


File.open("/Users/danicuki/Desktop/tw/loteria.txt", "r").readlines.each do |l|
  notas = l.split(" ").map {|n| n.to_f}
  puts "#{escala[notas[0] % 19]} #{(44100 * (1 / (2 ** (notas[1] % 5)))).to_i}"
end