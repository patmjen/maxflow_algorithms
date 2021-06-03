function varargout = mmeshgrid(sz)
	a = repmat([1:sz(1)]',1,sz(2));
	b = repmat([1:sz(2)],sz(1),1);
	if(nargout==1)
		varargout{1} = cat(3,a,b);
	end
	if(nargout==2)
		varargout{1} = a;
		varargout{2} = b;
	end
end