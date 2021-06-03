function A = mmin(A, dims)
   for i=1:length(dims)
   	A = min(A,[],dims(i));
   end
end