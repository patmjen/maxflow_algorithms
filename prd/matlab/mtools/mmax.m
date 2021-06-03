function A = mmax(A, dims)
   for i=1:length(dims)
   	A = max(A,[],dims(i));
   end
end