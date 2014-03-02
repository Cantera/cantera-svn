function v = child(x, loc)
% Get the child of an XML_Node instance
%
% :param x:
%     Instance of class :mat:func:`XML_Node`
% :param loc:
%     String loc to search for.
% :return:
%     Instance of class :mat:func:`XML_Node`
%

id = ctmethods(10, 6, x.id, loc);
v = XML_Node('', '', id);
