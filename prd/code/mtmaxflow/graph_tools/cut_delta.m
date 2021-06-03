%
% This function takes two different graph labellings at returns
% the difference in objective value
%
% Used to verify that a solution is global by comparing it to 
% another solution known to be global
%
% Petter Strandmark
%
function d = cut_delta(E,T,M,N, C1, C2)
    C = C1 - C2;
    val1 = 0;
    val2 = 0;
    for i = find(C ~= 0)'

        neigh = [i+1 i-1 i+M i-M i+M*N i-M*N];
        neigh( neigh>M*N ) = [];
        neigh( neigh<1 ) = [];
        
%         neigh = union(find(E(:,i)), find(E(i,:)) );
        
        if C1(i)==0 
            val1 = val1 + T(i,2);
            
            for j = neigh
                if C1(j)==1
                    val1 = val1 + E(i,j);
                end
            end   
        else
            val1 = val1 + T(i,1);
            
            for j = neigh
                if C1(j)==0
                    val1 = val1 + E(j,i);
                end
            end 
        end
        
        if C2(i)==0 
            val2 = val2 + T(i,2);
            
            for j = neigh
                if C2(j)==1
                    val2 = val2 + E(i,j);
                end
            end  
        else
            val2 = val2 + T(i,1);
            
            for j = neigh
                if C2(j)==0
                    val2 = val2 + E(j,i);
                end
            end  
        end
        
%         d = (val1 - val2)
    end
    d = abs(val1 - val2);
end