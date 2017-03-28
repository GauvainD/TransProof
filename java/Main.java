import org.neo4j.graphdb.GraphDatabaseService;
import org.neo4j.graphdb.Transaction;
import org.neo4j.graphdb.Node;
import org.neo4j.graphdb.Label;
import org.neo4j.graphdb.factory.GraphDatabaseFactory;
import org.neo4j.graphdb.factory.GraphDatabaseSettings;
import org.neo4j.unsafe.batchinsert.BatchInserter;
import java.util.Map;

import java.io.File;
import java.util.concurrent.Semaphore;

public class Main {
    private static GraphDatabaseService graphDB = null;
    private static Semaphore sem = new Semaphore(1, true);
    private static long numRef = 0;
    private Transaction tx;
    public long numTrans = 0;

    public static long createNode(BatchInserter inserter, Map<String, Object> properties, String label) {
        Label lab = Label.label(label);
        return inserter.createNode(properties, lab);
    }

    public Main() {
        init();
        tx = null;
    }

    public boolean startTransaction() {
        if (tx == null) {
            try {
                //Transactions are thread safe but
                //huge risk of deadlock and thus of
                //transaction rollback with neo4j
                //policy (ex : thead 1 adds a->b
                //and thread 2 adds b->a)
                sem.acquire();
                tx = graphDB.beginTx();
                //System.err.println("Start transaction : "+numTrans);
                numTrans++;
                return true;
            } catch (InterruptedException e) {
                if (tx != null) {
                    tx.close();
                    tx = null;
                }
                e.printStackTrace();
                return false;
            }
            //Exception type is not documented by Neo4j :-(
            catch (Exception e) {
                if (tx != null) {
                    tx.close();
                    tx = null;
                }
                e.printStackTrace();
                return false;
            } finally {

            }
        }
        return true;
    }

    public void endTransaction() {
        if (tx != null) {
            tx.success();
            tx.close();
            tx = null;
            //System.err.println("End transaction "+numTrans);
            numTrans--;
            sem.release();
        }
    }

    private static void init() {
        try {
            sem.acquire();
            numRef++;
            if (graphDB == null) {
                graphDB = new GraphDatabaseFactory().newEmbeddedDatabaseBuilder(new File("test.db")).setConfig(GraphDatabaseSettings.pagecache_memory, "1M").newGraphDatabase();
                //graphDB = new GraphDatabaseFactory().newEmbeddedDatabase(new File("test.db"));
            }
            sem.release();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static void finish() {
        try {
            sem.acquire();
            if (numRef <= 1 && graphDB != null) {
                System.out.println("finish");
                graphDB.shutdown();
                graphDB = null;
            } else {
                System.out.println("finishNum");
                numRef--;
            }
            sem.release();
            System.err.println("closed");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public Node makeNode() {
        Node n = graphDB.createNode();
        return n;
    }

    public static void main(String args[]) {
        new Thread() {
            @Override
            public void run() {
                Main m = new Main();
                m.startTransaction();
                m.makeNode();
                m.endTransaction();
                m.finish();
            }
        } .start();
        new Thread() {
            @Override
            public void run() {
                Main m = new Main();
                m.finish();
            }
        } .start();
        System.out.println("Hello World !");
    }
}
