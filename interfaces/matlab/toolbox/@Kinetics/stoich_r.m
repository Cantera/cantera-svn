function nu_r = stoich_r(a,species,rxns)
% stoich_r  Reactant stoichiometric coefficients.
%
%    nu = stoich_r(a)
%
%        Returns a sparse matrix of all reactant stoichiometric
%        coefficients. The matrix element nu(k,i) is the
%        stoichiometric coefficient of species k as a reactant in
%        reaction i.
%
%    nu = stoich_r(a, species, rxns)
%
%        Returns a sparse matrix the same size as above, but
%        containing only entries for the specified species and
%        reactions. For example, stoich_r(a,3,[1 3 5 7]) returns a
%        sparse matrix containing only the coefficients for species 3
%        in reactions 1, 3, 5, and 7.
%
%    See also: stoich_p, stoich_net.
%
nsp = nTotalSpecies(a);
nr =nReactions(a);
b = sparse(nsp,nr);
f = @kinetics_get;
if nargin == 1
    kvals = 1:nsp;
    ivals = 1:nr;
elseif nargin == 3
    kvals = species;
    ivals = rxns;
else
    error('Syntax error. type ''help stoich_r'' for more information.')
end

for k = kvals
    for i = ivals
        nu = feval(f,a.id,5,i,k);
        if nu ~= 0.0
            b(k,i) = nu;
        end
    end
end
nu_r = b;
