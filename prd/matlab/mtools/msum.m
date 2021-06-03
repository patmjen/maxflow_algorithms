function A = msum(A, dims)
   for i=1:length(dims)
   	A = sum(A,dims(i));
   end
   A = squeeze(A);
end