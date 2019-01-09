##
## Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
## All rights reserved.
## See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
##

#!/usr/bin/ruby

require 'getoptlong'

class RcsSpec
    def initialize(lastRevLogCmd, lastRevRegExp)
        @lastRevLogCmd = lastRevLogCmd
        @lastRevRegExp = lastRevRegExp
    end
    attr_reader :lastRevLogCmd, :lastRevRegExp
end # class RcsSpec

# Return the last revision number of the current working copy
def revNum(rcsType, workDir)
    rcsType = rcsType.downcase
    supportedRcsHash = Hash.new
    supportedRcsHash["bzr"] = RcsSpec.new("bzr revno", /^([0-9]+)/)
    supportedRcsHash["svn"] = RcsSpec.new("svnversion", /^([0-9]+)/)
    supportedRcsHash["git"] = RcsSpec.new("git rev-parse --short HEAD", /^([0-9a-zA-Z]+)/)

    if not supportedRcsHash.has_key?(rcsType) then
        puts "Revision control system not supported '#{rcsType}'"
        return ""
    end

    if not File.exists?(workDir) then
        puts "Directory '#{workDir}' does not exist"
        return ""
    end
    Dir.chdir(workDir)

    # Retrieve the last revision number
    rcsSpec = supportedRcsHash[rcsType]
    log_io = IO.popen(rcsSpec.lastRevLogCmd)
    matchData =  rcsSpec.lastRevRegExp.match(log_io.readlines.join)
    if matchData.size > 0 then
        matchData[1]
    else
        puts "Could not find revision number"
        ""
    end
end # def revNum()

# Parse command line
opts = GetoptLong.new(
#   ['--help', '-h', GetoptLong::NO_ARGUMENT],
    ['--rcs', '-s', GetoptLong::OPTIONAL_ARGUMENT],
    ['--workdir', '-d', GetoptLong::OPTIONAL_ARGUMENT])

rcsType = "bzr"
workDir = "."
opts.each do |opt, arg|
    case opt
    when '--rcs'
        rcsType = arg
    when '--workdir'
        workDir = arg
    end
end

# Execute
puts revNum(rcsType, workDir)
