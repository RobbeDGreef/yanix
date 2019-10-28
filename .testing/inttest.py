
integer = 123456

x = 0
while integer > 0:
	x = x*10 + integer % 10
	integer /= 10

integer = x

while integer != 0:
	print(integer % 10)
	integer /= 10
