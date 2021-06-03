function ff = fixpath(ff)
% ff = fixpath(ff)
%
% resolve relative positions within ff
% e.g. fixpath('A/../B/') outputs 'B/'

while 1
	p = strfind(ff,'..');
	if length(p)==0
		break;
	end
	p = p(1);
	r1 = strfind(ff(1:p-2),'/');
	r2 = strfind(ff(1:p-2),'\');
	r = max([r1 r2]);
	ff = [ff(1:r) ff(p+3:end)];
end
