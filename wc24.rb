#!/usr/bin/env ruby
require 'socket'

debug = true

@config = {
	:port => 8024,
	:root => './html',
	:index => ['index.html']
}

def ff path
	return File.join(File.expand_path(@config[:root]), path) || nil
end

def index_file path
	fn = ff path
	if File.directory? fn
		%w(index.html index.txt).find {|f|
			if File.file? File.join fn, f
				puts "Index file: #{fn}"
				return File.join fn, f
			end
		}
	end
	nil
end

def folder_listing path
	# template=nil

	o = "<html>"
	o << "<head>"
	o << "<title>Folder listing: #{path}</title>"
	o << "</head>\n"
	o << "<body>\n"
	o << "<ul>\n"
	o << Dir["#{ff(path)}/*"].map { |f|
		# puts "- f: #{f}"
		if File.directory? File.expand_path f
			[1, "<li><a href='#{path}/#{File.basename f}'>#{File.basename f}/</a></li>"]
		elsif File.file? File.expand_path f
			[2, "<li><a href='#{path}/#{File.basename f}'>#{File.basename f}</a></li>"]
		else
			[2, "<li><a href='#'>404 - #{File.basename f}</a></li>"]
		end
	}.
		sort{|a,b| a[0]<=>b[0]}.
		map{|x| x[1]}.
		join("\n") || ( "<p>Empty dir</p>\n" )
	o << "</ul>\n"
	o << "</body>"
	o << "\n"
	return o
end

def error s, code=500, msg=nil
	$stdout.puts "[err #{code}: #{msg}]"
	s.write "Error: #{code}\n"
	s.write "#{msg}\n" if msg
	s.close
	s.exit
end

server = TCPServer.open(@config[:port])
loop {
	Thread.start(server.accept) do |s|
		# request buffer
		# $stderr.reopen $stdout
		r = s.read
		request = r.split(/\r?\n\r?\n/)[0]
		# error s, 500, "Krivi request, nema 2 endline-a" if not r =~ /\r?\n\r?\n/
		puts request.inspect if debug
		# headers = request.split(/\r?\n/).map{ |x| Hash[*x.collect { |v| [v.split ': '] }.flatten] }
		if x = request.match(/GET (.+) HTTP\/1.[01]/)
			path = x[1]
			# puts "Path (raw): #{path}"
			if path[/^./] == '/'
				puts "Path: #{path}"
				puts "Local path (ff path): #{ff(path).inspect}"
				puts ((File.exist? ff path) ? "Postoji" : "Ne postoji") + " " + ((File.directory? ff path) ? "(Dir)" : "")
				if File.exists? ff path
					puts "Path postoji na FS"
					if File.directory? ff path
						# puts "path je dir"
						if indexfname=index_file(path)
							s.write File.read indexfname
							puts "Index file poslan"
						else
							puts "Folder listing gen"
							s.write folder_listing path
							puts "Poslan"
						end
					elsif File.file? ff path
						puts "path je file"
						puts "ff path: #{ff path}"
						s.write File.read ff path
						puts "File je poslan"
					else
						error s, 404, "Path nepostoji na serveru"
					end
				end
				s.close
			else
				error s, 404, "path ne pocinje sa '/'"
			end
		end
		s.write "OK\n"
	end
}
