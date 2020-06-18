package com._4paradigm.rtidb.client.impl;

import com._4paradigm.rtidb.client.KvIterator;
import com._4paradigm.rtidb.client.TabletException;
import com._4paradigm.rtidb.client.schema.ColumnDesc;
import com._4paradigm.rtidb.client.schema.RowView;
import com._4paradigm.rtidb.ns.NS;
import com._4paradigm.rtidb.utils.Compress;
import com.google.protobuf.ByteString;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

public class RowKvIterator implements KvIterator {
    private List<ScanResultParser> dataList = new ArrayList<>();
    // no copy
    private ByteBuffer slice = null;
    private int length;
    private List<ColumnDesc> schema;
    private int count = 0;
    private int iter_count = 0;
    private long key;
    private NS.CompressType compressType = NS.CompressType.kNoCompress;
    private RowView rv;

    public RowKvIterator(ByteString bs, List<ColumnDesc> schema, int count) {
        this.dataList.add(new ScanResultParser(bs));
        this.count = count;
        next();
        this.schema = schema;
        rv = new RowView(schema);
    }

    public RowKvIterator(List<ByteString> bsList, List<ColumnDesc> schema, int count) {
        for (ByteString bs : bsList) {
            if (!bs.isEmpty()) {
                this.dataList.add(new ScanResultParser(bs));
            }
        }
        this.count = count;
        next();
        this.schema = schema;
        rv = new RowView(schema);
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }

    public NS.CompressType getCompressType() {
        return compressType;
    }

    public void setCompressType(NS.CompressType compressType) {
        this.compressType = compressType;
    }

    public List<ColumnDesc> getSchema() {
        return schema;
    }

    public boolean valid() {
        return slice != null;
    }

    public long getKey() {
        return key;
    }

    @Override
    public String getPK() {
        return null;
    }

    // no copy
    public ByteBuffer getValue() {
        return slice;
    }

    public Object[] getDecodedValue() throws TabletException {
        if (schema == null) {
            throw new TabletException("get decoded value is not supported");
        }
        Object[] row = new Object[schema.size()];
        getDecodedValue(row, 0, row.length);
        return row;
    }

    public void next() {
        if (count <= iter_count) {
            slice = null;
            return;
        }
        int maxTsIndex = -1;
        long ts = -1;
        for (int i = 0; i < dataList.size(); i++) {
            ScanResultParser queryResult = dataList.get(i);
            if (queryResult.valid() && queryResult.GetTs() > ts) {
                maxTsIndex = i;
                ts = queryResult.GetTs();
            }
        }
        if (maxTsIndex >= 0) {
            ScanResultParser queryResult = dataList.get(maxTsIndex);
            key = queryResult.GetTs();
            slice = queryResult.fetchData().slice();
        } else {
            slice = null;
        }
        iter_count++;
    }

    @Override
    public void getDecodedValue(Object[] row, int start, int length) throws TabletException {
        if (schema == null) {
            throw new TabletException("get decoded value is not supported");
        }
        if (compressType == NS.CompressType.kSnappy) {
            byte[] data = new byte[slice.remaining()];
            slice.get(data);
            byte[] uncompressed = Compress.snappyUnCompress(data);
            rv.read(ByteBuffer.wrap(uncompressed).order(ByteOrder.LITTLE_ENDIAN), row, start, length);
        } else {
            rv.read(slice.order(ByteOrder.LITTLE_ENDIAN), row, start, length);
        }

    }
}