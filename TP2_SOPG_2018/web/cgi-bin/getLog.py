#! /usr/bin/env python
# 	 SOPG 2018. TP 2.
#    Copyright (C) 2018  Ernesto Gigliotti.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
import json

print("Content-Type: text/html")
print("")

fp = open("/tmp/log.txt", 'r')
data = fp.read()
fp.close()

lines = iter(data.splitlines())

out = []

for l in lines:
	fields = l.split(",")
	dic = {"id":fields[0],"date":fields[1],"time":fields[2]}
	out.append(dic)

print(json.dumps(out))
