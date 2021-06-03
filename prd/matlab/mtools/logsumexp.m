function logSumA = logsumexp(logA,dim)
% LOGSUMEXP Computes logarithm of a sum of very small numbers.
%
% Synopis:
%    logSumA = logsumexp(logA)
%
% Description:
%  This function computes
%
%    logSumA = log( sum( A ) )
%
%  from logA = log(A), i.e. from logarithm of the summands where A [n x 1] 
%  is a vector of positive reals numbers. 
%
%  This function is equivalent to log(sum(exp( logA ))) . The function
%  logsumexp is useful if A are extremaly small numbers in which case 
%  log(sum(exp(logA))) would not work due to a finite precision of double.
%
% Examples:
%  logA = [-1000 -1000 -1000 -1000 -1000 -1000 -1000 -1000]
%  log(sum(exp(logA)))
%  logsumexp(logA)
%
%  logA = [-100 -100 -100 -100 -100 -100 -100 -100]
%  log(sum(exp(logA)))
%  logsumexp(logA)
%

if min(size(logA)) == 1
    sorted_logA = sort(logA(:));
else
    sorted_logA = sort(logA);    
end

logSumA = sorted_logA(1,:);

for i=2:size(sorted_logA,1)
    logSumA = sorted_logA(i,:) + log(1 + exp(logSumA - sorted_logA(i,:)));
end

return;
