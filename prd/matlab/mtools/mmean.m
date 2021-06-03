function A = mmin(A, dims)
	dims = sort(dims);
   for i=1:length(dims)
   	A = mean(A,dims(i));
   end
end