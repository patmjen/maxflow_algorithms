function print_time(f,strct,field,suffix)
if ~exist('suffix','var')
	suffix = '';
end
if(isfield(strct,field))
	v = strct.(field);
	if(isscalar(v))
		if(v<1)
			fprintf(f,['$%1.2f$s' suffix],v);
		elseif (v<10)
			fprintf(f,['$%2.1f$s' suffix],v);
		elseif (v<10*60)
			fprintf(f,['$%2.0f$s' suffix],v);
		else
			fprintf(f,['$%2.0f$min' suffix],v/60);
		end
	else
		fprintf(f,['%s' suffix],v);
	end
else
	fprintf(f,suffix);
end
end