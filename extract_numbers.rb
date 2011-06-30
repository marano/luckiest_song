require 'rubygems'
require 'nokogiri'

doc = Nokogiri::HTML(File.read './numbers.htm')
rows = doc.xpath('//tr')

rows[1, rows.size].each do |tr|
  p tr.xpath('./td')[2, 6].map(&:text).join(' ') 
end
