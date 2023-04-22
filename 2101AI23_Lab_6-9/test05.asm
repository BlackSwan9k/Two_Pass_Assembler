		ldc 15 ;this is a comment
		ldc 16
		a2sp
loop:		ldl 1
		adj 1
		ldl 3
		stnl 0x738
		ldl 3
		adc 1
		stl 3
		ldl 0
		adc 1
		brz loop1
label1: 	add
		ldc 13
		brz label1
		ldc 0x342
		add
loop1: 	call loop