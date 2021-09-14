import os
import sys
from collections import namedtuple
from typing import Union, T, Tuple, TypeVar, Generic

import numpy as np
import snappy

T = TypeVar('T')
try:
    from numpy.typing import ArrayLike, DTypeLike
except ModuleNotFoundError:
    # NumPy version is too old
    DTypeLike = TypeVar('DTypeLike')
    class ArrayLike(Generic[T]):
        pass


def code_to_type(type_code: int) -> np.dtype:
    """Convert numpy datype to type code."""
    if type_code == 0:
        return np.dtype('uint8')
    elif type_code == 1:
        return np.dtype('int8')
    elif type_code == 2:
        return np.dtype('uint16')
    elif type_code == 3:
        return np.dtype('int16')
    elif type_code == 4:
        return np.dtype('uint32')
    elif type_code == 5:
        return np.dtype('int32')
    elif type_code == 6:
        return np.dtype('uint64')
    elif type_code == 7:
        return np.dtype('int64')
    elif type_code == 8:
        return np.dtype('float')
    elif type_code == 9:
        return np.dtype('double')
    else:
        raise ValueError(f'Invalid type code. Got {type_code}')


def type_to_code(dtype: DTypeLike) -> int:
    """Convert numpy datype to type code."""
    if dtype == np.dtype('uint8'):
        return 0
    elif dtype == np.dtype('int8'):
        return 1
    elif dtype == np.dtype('uint16'):
        return 2
    elif dtype == np.dtype('int16'):
        return 3
    elif dtype == np.dtype('uint32'):
        return 4
    elif dtype == np.dtype('int32'):
        return 5
    elif dtype == np.dtype('uint64'):
        return 6
    elif dtype == np.dtype('int64'):
        return 7
    elif dtype == np.dtype('float'):
        return 8
    elif dtype == np.dtype('double'):
        return 9
    else:
        raise ValueError(f'Invalid dtype. Got {dtype}')


def type_size(type_code: int) -> int:
    """Return size of type in bytes."""
    return code_to_type(type_code).itemsize


BkGraphHeader = namedtuple('BkGraphHeader', [
    'compressed',
    'captype',
    'tcaptype',
    'num_nodes',
    'num_term_arcs',
    'num_nbor_arcs',
])


BkGraph = namedtuple('BkGraph', [
    'num_nodes',
    'term_arcs',
    'nbor_arcs',
])


def _bk_term_arc_dtype(cap_type: DTypeLike) -> np.dtype:
    """Construct numpy dtype for terminal arcs given a capacity type."""
    return np.dtype([
        ('node', np.uint64),
        ('source_cap', cap_type),
        ('sink_cap', cap_type),
    ])


def _bk_nbor_arc_dtype(cap_type: DTypeLike) -> np.dtype:
    """Construct numpy dtype for neighbor arcs given a capacity type."""
    return np.dtype([
        ('from', np.uint64),
        ('to', np.uint64),
        ('cap', cap_type),
        ('rev_cap', cap_type),
    ])


BkQpboHeader = namedtuple('BkQpboHeader', [
    'compressed',
    'captype',
    'num_nodes',
    'num_unary_terms',
    'num_binary_terms',
])


BkQpbo = namedtuple('BkQpbo', [
    'num_nodes',
    'unary_terms',
    'binary_terms',
])


def _bk_unary_term_dtype(cap_type: DTypeLike) -> np.dtype:
    """Construct numpy dtype for unary terms given a capacity type."""
    return np.dtype([
        ('node', np.uint64),
        ('e0', cap_type),
        ('e1', cap_type),
    ])


def _bk_binary_term_dtype(cap_type: DTypeLike) -> np.dtype:
    """Construct numpy dtype for binary given a capacity type."""
    return np.dtype([
        ('i', np.uint64),
        ('j', np.uint64),
        ('e00', cap_type),
        ('e01', cap_type),
        ('e10', cap_type),
        ('e11', cap_type),
    ])


class BkGraphBuilder:
    """Helper class to build BK graphs."""

    def __init__(
        self,
        num_nodes: int = 0,
        expected_term_arcs: int = 0,
        expected_nbor_arcs: int = 0,
        term_cap_type: DTypeLike = np.int32,
        nbor_cap_type: DTypeLike = np.int32
    ):
        """
        Args:
            num_nodes: Number of initial graph nodes.
            expected_term_arcs: Expected number of terminal arcs.
            expected_nbor_arcs: Expected number of neighbor arcs.
            term_cap_type: Type of terminal arc capacity.
            nbor_cap_type: Type of neighbor arc capacity.
        """
        self.num_nodes = num_nodes
        self.num_term_arcs = 0
        self.num_nbor_arcs = 0

        self.term_arcs = np.empty(
            expected_term_arcs,
            dtype=_bk_term_arc_dtype(term_cap_type)
        )
        self.nbor_arcs = np.empty(
            expected_nbor_arcs,
            dtype=_bk_nbor_arc_dtype(nbor_cap_type)
        )


    def add_nodes(
        self,
        num_nodes: Union[int, ArrayLike[int]]
    ):
        """
        Add nodes to graph.

        Args:
            num_nodes: Number of nodes to add.
        """
        self.num_nodes += num_nodes


    def add_tedge(
        self,
        node: Union[int, ArrayLike[int]],
        source_cap: Union[T, ArrayLike[T]],
        sink_cap: Union[T, ArrayLike[T]]
    ):
        """
        Add terminal edge(s) to graph. Alias of `add_tedges`.
        Only allocates new space if needed.

        Args:
            node: index or array-like of indices with nodes.
            source_cap: value or array-like of values with source capacities.
            sink_cap: value or array-like of values with sink capacities.
        """
        self.add_tedges(node, source_cap, sink_cap)


    def add_tedges(
        self,
        nodes: Union[int, ArrayLike[int]],
        source_caps: Union[T, ArrayLike[T]],
        sink_caps: Union[T, ArrayLike[T]]
    ):
        """
        Add terminal edge(s) to graph.
        Only allocates new space if needed.

        Args:
            nodes: index or array-like of indices with nodes.
            source_caps: value or array-like of values with source capacities.
            sink_caps: value or array-like of values with sink capacities.
        """
        nodes = np.asarray(nodes).ravel()
        source_caps = np.asarray(source_caps).ravel()
        sink_caps = np.asarray(sink_caps).ravel()

        assert len(nodes) == len(source_caps)
        assert len(nodes) == len(sink_caps)

        num_new_arcs = len(nodes)
        res_capacity = len(self.term_arcs) - self.num_term_arcs
        if res_capacity > 0:
            # We can fit some new arcs in the existing array so just add them
            to_add = min(num_new_arcs, res_capacity)
            new_num_term_arcs = self.num_term_arcs + to_add
            idx = slice(self.num_term_arcs, new_num_term_arcs)

            self.term_arcs['node'][idx] = nodes[:to_add]
            self.term_arcs['source_cap'][idx] = source_caps[:to_add]
            self.term_arcs['sink_cap'][idx] = sink_caps[:to_add]

            self.num_term_arcs = new_num_term_arcs

            nodes = nodes[to_add:]
            source_caps = source_caps[to_add:]
            sink_caps = sink_caps[to_add:]

        self._append_tedges(nodes, source_caps, sink_caps)


    def _append_tedges(
        self,
        nodes: Union[int, ArrayLike[int]],
        source_caps: Union[T, ArrayLike[T]],
        sink_caps: Union[T, ArrayLike[T]]
    ):
        """
        Append terminal edge(s) to graph. Assumes no capacity is left.

        Args:
            nodes: index or array-like of indices with nodes.
            source_caps: value or array-like of values with source capacities.
            sink_caps: value or array-like of values with sink capacities.
        """
        num_new_edges = len(nodes)
        if num_new_edges == 0:
            # Nothing to add
            return
        assert self.num_term_arcs == len(self.term_arcs)
        new_arcs = np.empty(num_new_edges, dtype=self.term_arcs.dtype)
        new_arcs['node'] = nodes
        new_arcs['source_cap'] = source_caps
        new_arcs['sink_cap'] = sink_caps

        self.term_arcs = np.concatenate([self.term_arcs, new_arcs])
        self.num_term_arcs += num_new_edges


    def add_edge(
        self,
        from_: Union[int, ArrayLike[int]],
        to: Union[int, ArrayLike[int]],
        cap: Union[T, ArrayLike[T]],
        rev_cap: Union[T, ArrayLike[T]]
    ):
        """
        Add directed neighbor edge(s) to graph. Alias of `add_edges`.
        Only allocates new space if needed.

        Args:
            from_: index or array-like of indices with `from` nodes.
            to: index or array-like of indices with `to` nodes.
            cap: value or array-like of values with forward capacities.
            rev_cap: value or array-like of values with reverse capacities.
        """
        self.add_edges(from_, to, cap, rev_cap)


    def add_edges(
        self,
        froms: Union[int, ArrayLike[int]],
        tos: Union[int, ArrayLike[int]],
        caps: Union[T, ArrayLike[T]],
        rev_caps: Union[T, ArrayLike[T]]
    ):
        """
        Add directed neighbor edge(s) to graph.
        Only allocates new space if needed.

        Args:
            froms: index or array-like of indices with `from` nodes.
            tos: index or array-like of indices with `to` nodes.
            caps: value or array-like of values with forward capacities.
            rev_caps: value or array-like of values with reverse capacities.
        """
        froms = np.asarray(froms).ravel()
        tos = np.asarray(tos).ravel()
        caps = np.asarray(caps).ravel()
        rev_caps = np.asarray(rev_caps).ravel()

        assert len(froms) == len(tos)
        assert len(froms) == len(caps)
        assert len(froms) == len(rev_caps)

        num_new_arcs = len(froms)
        res_capacity = len(self.nbor_arcs) - self.num_nbor_arcs
        if res_capacity > 0:
            # We can fit some new arcs in the existing array so just add them
            to_add = min(num_new_arcs, res_capacity)
            new_num_nbor_arcs = self.num_nbor_arcs + to_add
            idx = slice(self.num_nbor_arcs, new_num_nbor_arcs)

            self.nbor_arcs['from'][idx] = froms[:to_add]
            self.nbor_arcs['to'][idx] = tos[:to_add]
            self.nbor_arcs['cap'][idx] = caps[:to_add]
            self.nbor_arcs['rev_cap'][idx] = rev_caps[:to_add]

            self.num_nbor_arcs = new_num_nbor_arcs

            froms = froms[to_add:]
            tos = tos[to_add:]
            caps = caps[to_add:]
            rev_caps = rev_caps[to_add:]

        self._append_edges(froms, tos, caps, rev_caps)


    def _append_edges(
        self,
        froms: Union[int, ArrayLike[int]],
        tos: Union[int, ArrayLike[int]],
        caps: Union[T, ArrayLike[T]],
        rev_caps: Union[T, ArrayLike[T]]
    ):
        """
        Append neighbor edge(s) to graph. Assumes no capacity is left.

        Args:
            froms: index or array-like of indices with `from` nodes.
            tos: index or array-like of indices with `to` nodes.
            caps: value or array-like of values with forward capacities.
            rev_caps: value or array-like of values with reverse capacities.
        """
        num_new_edges = len(froms)
        if num_new_edges == 0:
            # Nothing to add
            return
        assert self.num_nbor_arcs == len(self.nbor_arcs)
        new_arcs = np.empty(num_new_edges, dtype=self.nbor_arcs.dtype)
        new_arcs['from'] = froms
        new_arcs['to'] = tos
        new_arcs['cap'] = caps
        new_arcs['rev_cap'] = rev_caps

        self.nbor_arcs = np.concatenate([self.nbor_arcs, new_arcs])
        self.num_nbor_arcs += num_new_edges


    def bk_graph(self) -> BkGraph:
        """
        Returns BkGraph representation of the graph.
        """
        return BkGraph(
            num_nodes=self.num_nodes,
            term_arcs=self.term_arcs[:self.num_term_arcs],
            nbor_arcs=self.nbor_arcs[:self.num_nbor_arcs],
        )


    def save(self, fname: str, compress: bool = False):
        """
        Save graph as binary BK file.

        For large graphs it is an advantage to use compression as it makes
        reading the graph faster. However, graphs larger than 4GB cannot be
        compressed due to limitations of the used compression library (snappy).

        Args:
            fname: Name of binary BK file to save to.
            compress: Whether to use compression when saving.
        """
        write_bbk(fname, self.bk_graph(), compress=compress)


def read_bbk_header(file_handle) -> BkGraphHeader:
    """
    Read header information of binary BK file.

    Args:
        file_handle: File handle to open binary BK file.

    Return:
        out: BkGraphHeader with header information.
    """
    header = file_handle.read(3).decode()
    if len(header) != 3 or header.lower() != 'bbq':
        raise ValueError('file header is invalid')

    compressed = header.islower()

    types_bytes = file_handle.read(2)  # Read type codes (2 * uint8)
    captype = types_bytes[0]
    tcaptype = types_bytes[1]

    sizes_bytes = file_handle.read(3 * 8)  # Read sizes (3 * uint64)

    # Num. nodes, num. term. edges, num. nbor. edges
    sizes = [int.from_bytes(sizes_bytes[i:i+8], sys.byteorder)
             for i in range(0, 3 * 8, 8)]

    return BkGraphHeader(
        compressed=compressed,
        captype=captype,
        tcaptype=tcaptype,
        num_nodes=sizes[0],
        num_term_arcs=sizes[1],
        num_nbor_arcs=sizes[2]
    )


def read_bbk_sizes(fname: str) -> Tuple[int, int]:
    """
    Read size of graph in binary BK file.

    Args:
        fname: File name of binary BK file.

    Return:
        (num_nodes, num_edges): Tuple with number of nodes and number of edges.
    """
    with open(fname, 'rb') as f:
        header = read_bbk_header(f)

    return header.num_nodes + 2, header.num_term_arcs + header.num_nbor_arcs


def read_bbk(fname: str) -> BkGraph:
    """
    Read binary BK file.

    Args:
        fname: File name of binary BK file.

    Return:
        graph: Graph as BkGraph.
    """
    with open(fname, 'rb') as f:
        header = read_bbk_header(f)

        if not header.compressed:
            # Just read in the data
            term_arc_size = 8 + 2 * type_size(header.tcaptype)
            nbor_arc_size = 16 + 2 * type_size(header.captype)

            term_arcs_bytes = f.read(term_arc_size * header.num_term_arcs)
            nbor_arcs_bytes = f.read(nbor_arc_size * header.num_nbor_arcs)
        else:
            # Need to uncompress first
            term_arc_size = int.from_bytes(f.read(8), sys.byteorder)
            term_arcs_bytes = snappy.uncompress(f.read(term_arc_size))

            nbor_arc_size = int.from_bytes(f.read(8), sys.byteorder)
            nbor_arcs_bytes = snappy.uncompress(f.read(nbor_arc_size))

    term_arc_type = _bk_term_arc_dtype(code_to_type(header.tcaptype))
    nbor_arc_type = _bk_nbor_arc_dtype(code_to_type(header.captype))

    term_arcs = np.frombuffer(term_arcs_bytes, dtype=term_arc_type)
    nbor_arcs = np.frombuffer(nbor_arcs_bytes, dtype=nbor_arc_type)

    return BkGraph(
        num_nodes=header.num_nodes,
        term_arcs=term_arcs,
        nbor_arcs=nbor_arcs,
    )


def write_bbk(fname: str, graph: BkGraph, compress: bool = False):
    """
    Write graph to binary BK file.

    For large graphs it is an advantage to use compression as it makes reading
    the graph faster. However, graphs larger than 4GB cannot be compressed due
    to limitations of the used compression library (snappy).

    Args:
        fname: Name of binary BK file to save to.
        graph: BkGraph to save.
        compress: Whether to use compression when saving.
    """
    header = b'bbq' if compress else b'BBQ'
    with open(fname, 'wb') as f:
        # Write header
        f.write(header)

        # Write data types
        np.uint8(type_to_code(graph.nbor_arcs['cap'].dtype)).tofile(f)
        np.uint8(type_to_code(graph.term_arcs['source_cap'].dtype)).tofile(f)

        # Write sizes
        np.uint64([
            graph.num_nodes,
            len(graph.term_arcs),
            len(graph.nbor_arcs),
        ]).tofile(f)

        # Write graph data
        if not compress:
            graph.term_arcs.tofile(f)
            graph.nbor_arcs.tofile(f)
        else:
            compressed_term_arcs = snappy.compress(graph.term_arcs.tobytes())
            compressed_nbor_arcs = snappy.compress(graph.nbor_arcs.tobytes())
            np.uint64(len(compressed_term_arcs)).tofile(f)
            f.write(compressed_term_arcs)
            np.uint64(len(compressed_nbor_arcs)).tofile(f)
            f.write(compressed_nbor_arcs)


def read_bqpbo_header(file_handle) -> BkQpboHeader:
    """
    Read header information of binary QPBO file.

    Args:
        file_handle: File handle to open binary QPBO file.

    Return:
        out: BkQpboHeader with header information.
    """
    header = file_handle.read(5).decode()
    if len(header) != 5 or header.lower() != 'bqpbo':
        raise ValueError('file header is invalid')

    compressed = header.islower()

    captype = file_handle.read(1)[0]  # Read type codes (1 * uint8)
    sizes_bytes = file_handle.read(3 * 8)  # Read sizes (3 * uint64)

    # Num. vars., num. unary terms, num. binary terms
    sizes = [int.from_bytes(sizes_bytes[i:i+8], sys.byteorder)
             for i in range(0, 3 * 8, 8)]

    return BkQpboHeader(
        compressed=compressed,
        captype=captype,
        num_nodes=sizes[0],
        num_unary_terms=sizes[1],
        num_binary_terms=sizes[2]
    )


def read_bqpbo_sizes(fname: str) -> Tuple[int, int]:
    """
    Read size of resulting graph in binary QPBO file.

    Args:
        fname: File name of binary QPBO file.

    Return:
        (num_nodes, num_edges): Tuple with number of nodes and number of edges.
    """
    with open(fname, 'rb') as f:
        header = read_bqpbo_header(f)

    return (2 * header.num_nodes + 2,
            2 * header.num_unary_terms + 2 * header.num_binary_terms)


def read_bqpbo(fname: str) -> BkQpbo:
    """
    Read binary QPBO file.

    Args:
        fname: File name of binary QPBO file.

    Return:
        graph: QPBO problem as BkQpbo.
    """
    with open(fname, 'rb') as f:
        header = read_bqpbo_header(f)

        if not header.compressed:
            # Just read in the data
            unary_term_size = 8 + 2 * type_size(header.captype)
            binary_term_size = 18 + 4 * type_size(header.captype)

            unary_bytes = f.read(unary_term_size * header.num_unary_terms)
            binary_bytes = f.read(binary_term_size * header.num_binary_terms)
        else:
            # Need to uncompress first
            unary_terms_size = int.from_bytes(f.read(8), sys.byteorder)
            unary_bytes = snappy.uncompress(f.read(unary_terms_size))

            binary_terms_size = int.from_bytes(f.read(8), sys.byteorder)
            binary_bytes = snappy.uncompress(f.read(binary_terms_size))

    unary_term_type = _bk_unary_term_dtype(code_to_type(header.captype))
    binary_term_type = _bk_binary_term_dtype(code_to_type(header.captype))

    unary_terms = np.frombuffer(unary_bytes, dtype=unary_term_type)
    binary_terms = np.frombuffer(binary_bytes, dtype=binary_term_type)

    return BkQpbo(
        num_nodes=header.num_nodes,
        unary_terms=unary_terms,
        binary_terms=binary_terms,
    )


def write_bqpbo(fname: str, qpbo: BkQpbo, compress: bool = False):
    """
    Write QPBO problem to binary QPBO file.

    For large problems it is an advantage to use compression as it makes
    reading the problem faster. However, problems larger than 4GB cannot be
    compressed due to limitations of the used compression library (snappy).

    Args:
        fname: Name of binary QPBO file to save to.
        qpbo: BkQpbo to save.
        compress: Whether to use compression when saving.
    """
    header = b'bqpbo' if compress else b'BQPBO'
    with open(fname, 'wb') as f:
        # Write header
        f.write(header)

        # Write data types
        np.uint8(type_to_code(qpbo.unary_terms['e0'].dtype)).tofile(f)

        # Write sizes
        np.uint64([
            qpbo.num_nodes,
            len(qpbo.unary_terms),
            len(qpbo.binary_terms)
        ]).tofile(f)

        # Write problem data
        if not compress:
            qpbo.unary_terms.tofile(f)
            qpbo.binary_terms.tofile(f)
        else:
            compressed_unary = snappy.compress(qpbo.unary_terms.tobytes())
            compressed_binary = snappy.compress(qpbo.binary_terms.tobytes())
            np.uint64(len(compressed_unary)).tofile(f)
            f.write(compressed_unary)
            np.uint64(len(compressed_binary)).tofile(f)
            f.write(compressed_binary)


def main(argv):
    files = [f for f in os.listdir() if f.endswith('.bbk')]
    print('file_name,num_nodes,num_edges')
    for f in files:
        num_nodes, num_edges = read_bbk_sizes(f)
        print(f, num_nodes, num_edges, sep=',')


if __name__ == '__main__':
    main(sys.argv)

