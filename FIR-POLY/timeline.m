function [t, mt, w] = timeline (len, stretch)


if (stretch < 0 || stretch > 1)
	error ("stretch out of range [0, 1].\n");
	endif

# Mofified time

x_w = 0.2 ;
x = [-0.5 -0.2 0.2 0.5];
y = [-0.5 -0.4 -0.1 0.5];

p = polyfit (x, y, 4);

t = linspace (-0.5, 0.5, 100);
y = polyval (p,t);
plot (t, y)
return

mt = ((1 - stretch) * t + stretch  * t.^2) - 0.5 ;


# Basically a Nuttall indow
ak = [0.42323 0.49755 0.07922] ;

w = zeros (1, len) ;

k = 0 ;
for a = ak,
	w = w + a * cos (2 * pi * k * mt) ;
	k += 1 ;
	endfor

w = fliplr (w / len) ;

# Linear t

t = linspace (-(len-2)/2, (len-2)/2, len) / len ;


end

# Do not edit or modify anything in this comment block.
# The following line is a file identity tag for the GNU Arch
# revision control system.
#
# arch-tag: 830f701e-5e9c-478a-a929-609513d4b1a6


