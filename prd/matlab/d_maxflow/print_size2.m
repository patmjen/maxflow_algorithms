function print_size2(f,strct,field1,field2,suffix)
if(isfield(strct,field1) && isfield(strct,field2))
	v1 = strct.(field1);
	v2 = strct.(field2);
	if(isscalar(v1) && isscalar(v2))
		if(v1<10)
			fprintf(f,'$%2.1f{+}%2.1f$MB',v1,v2);
		elseif(v1<512)
			fprintf(f,'$%3.0f{+}%3.0f$MB',v1,v2);
		else
			fprintf(f,'$%2.1f{+}%2.1f$GB',v1/1024,v2/1024);
		end
	else
		fprintf(f,'%s+%s',v1,v2);
	end
else
end
fprintf(f,suffix);
end
