function [i j k] = multi_ind2sub(ssz)

if(size(ssz,1)==1)
	ind = invcum(ssz);
	start = accumarray(ind(:),1:length(ind),[max(ind) 1],@min)';
	ramp = [1:length(ind)]-start(ind)+1;
	
	i = ramp;
	j = ind;
end

if(size(ssz,1)==2)
	
	Ssz =ssz(1,:).*ssz(2,:);
	k = invcum(Ssz);
	
	r1 = multi_rep(ssz(1,:),ssz(2,:));
	
	[i j] = multi_ind2sub(r1);
	
	start = accumarray(k(:),j,[max(k) 1],@min)';
	j = j-start(k)+1;
end

end