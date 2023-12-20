##
## Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
## All rights reserved.
## See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
##

#!/usr/bin/ruby

if ARGV.empty?
    puts "Error: no version argument"
    return
end

version_array = ARGV.first.split('.')
if version_array.size != 3
    puts "Error: wrong version format(maj.min.patch)"
    return
end

major = version_array[0]
minor = version_array[1]
patch = version_array[2]
puts "Major: #{major}"
puts "Minor: #{minor}"
puts "Patch: #{patch}"
script_dir_name = File.expand_path(File.dirname(__FILE__))

# ../version.pri
path_version_pri = "#{script_dir_name}/../version.pri"
version_pri = File.open(path_version_pri, "r").read
version_pri.sub!(/(MAYO_VERSION_MAJ\s*=\s*)\d+/, "\\1#{major}")
version_pri.sub!(/(MAYO_VERSION_MIN\s*=\s*)\d+/, "\\1#{minor}")
version_pri.sub!(/(MAYO_VERSION_PAT\s*=\s*)\d+/, "\\1#{patch}")
File.open(path_version_pri, "w").write(version_pri)
puts "Bumped #{path_version_pri}"
