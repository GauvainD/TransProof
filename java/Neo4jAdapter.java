import org.neo4j.graphdb.Label;
import org.neo4j.unsafe.batchinsert.BatchInserter;
import java.util.Map;

public class Neo4jAdapter {

    public static long createNode(BatchInserter inserter, Map<String, Object> properties, String label) {
        Label lab = Label.label(label);
        return inserter.createNode(properties, lab);
    }
}
