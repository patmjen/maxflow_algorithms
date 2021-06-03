function r = get_cell_field(array,field)
	r = cellfun(@(x) getcf(x,field),array);
end

function r = getcf(c,field)
if isfield(c,field)
 	r = c.(field);
else
	r = 0;
end
end