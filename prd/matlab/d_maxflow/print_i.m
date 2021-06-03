function print_i(f,strct,field,suffix)
if ~exist('suffix','var')
	suffix = '';
end
if(isfield(strct,field))
	v = strct.(field);
	if(isscalar(v))
		fprintf(f,['$%3.0f$' suffix],v);
	else
		fprintf(f,['%s' suffix],v);
	end
else
	fprintf(f,suffix);
end
