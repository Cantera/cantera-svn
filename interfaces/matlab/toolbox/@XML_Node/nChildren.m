function n = nChildren(root)
% Get the number of children of a XML_Node
%
% :param root:
%     Instance of class :mat:func:`XML_Node`
% :return:
%     Integer number of children of the input XML_Node
%

n = ctmethods(10, 10, root.id);
