#ifndef NEO4J_INSERTER_HPP
#define NEO4J_INSERTER_HPP

#include <jni.h>
#include <cstdio>
#include <map>
#include <iostream>

#include "filter_test.hpp"

struct jnienv
{
    JNIEnv *env;
    jclass boolean;
    jmethodID boolconst;
    jclass integer;
    jmethodID integerconst;
    jclass floatc;
    jmethodID floatconst;
    jmethodID put;
    jmethodID clear;
    jclass main;
    jmethodID makenode;
    jclass types;
    jmethodID withname;
    jmethodID createRelationship;
    jclass javalong;
    jmethodID longconstr;
};

class Neo4jInserter
{
protected :

    JavaVM *jvm;
    std::map<std::string, jobject> relTypes;
    std::vector<struct jnienv> envs;
    jobject inserter;
    jobject hashmap;
    int threads;
    bool firstRel;
    jstring resTrs;
    jstring label;

    void initEnv(int threadNum, JNIEnv *env)
    {
        if (envs.size() <= threadNum)
        {
            struct jnienv envstr;
            envstr.env = env;
            envstr.boolean = env->FindClass("java/lang/Boolean");
            envstr.boolconst = env->GetMethodID(envstr.boolean, "<init>", "(Z)V");
            envstr.integer = env->FindClass("java/lang/Long");
            envstr.integerconst = env->GetMethodID(envstr.integer, "<init>", "(J)V");
            envstr.floatc = env->FindClass("java/lang/Double");
            envstr.floatconst = env->GetMethodID(envstr.floatc, "<init>", "(D)V");
            jclass mapClass = env->FindClass("java/util/HashMap");
            envstr.put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
            envstr.clear = env->GetMethodID(mapClass, "clear", "()V");
            envstr.main = env->FindClass("Neo4jAdapter");
            envstr.makenode = env->GetStaticMethodID(envstr.main, "createNode", "(Lorg/neo4j/unsafe/batchinsert/BatchInserter;Ljava/util/Map;Ljava/lang/String;)J");
            envstr.types = env->FindClass("org/neo4j/graphdb/RelationshipType");
            envstr.withname = env->GetStaticMethodID(envstr.types, "withName", "(Ljava/lang/String;)Lorg/neo4j/graphdb/RelationshipType;");
            jclass batchinserter = env->FindClass("org/neo4j/unsafe/batchinsert/BatchInserter");
            envstr.createRelationship = env->GetMethodID(batchinserter, "createRelationship", "(JJLorg/neo4j/graphdb/RelationshipType;Ljava/util/Map;)J");
            envs.push_back(envstr);
        }
    }

    void initJVM()
    {
        JavaVMInitArgs vm_args;
        JavaVMOption* options = new JavaVMOption[9];
        options[0].optionString = "-Djava.class.path=build:lib/jetty-security-9.2.9.v20150224.jar:lib/neo4j-graph-matching-3.0.3.jar:lib/neo4j-resource-3.0.3.jar:lib/neo4j-shell-3.0.3.jar:lib/commons-digester-2.1.jar:lib/jetty-servlet-9.2.9.v20150224.jar:lib/neo4j-import-tool-3.0.3.jar:lib/jackson-jaxrs-1.9.13.jar:lib/parboiled-core-1.1.7.jar:lib/jline-2.12.jar:lib/neo4j-server-3.0.3.jar:lib/commons-lang3-3.3.2.jar:lib/asm-5.0.2.jar:lib/bcprov-jdk15on-1.53.jar:lib/parboiled-scala_2.11-1.1.7.jar:lib/server-api-3.0.3.jar:lib/neo4j-jmx-3.0.3.jar:lib/neo4j-consistency-check-3.0.3.jar:lib/jsr311-api-1.1.2.r612.jar:lib/commons-io-2.4.jar:lib/commons-logging-1.1.1.jar:lib/neo4j-common-3.0.3.jar:lib/neo4j-dbms-3.0.3.jar:lib/commons-lang-2.6.jar:lib/jetty-webapp-9.2.9.v20150224.jar:lib/neo4j-csv-3.0.3.jar:lib/neo4j-cypher-frontend-2.3-2.3.4.jar:lib/neo4j-graph-algo-3.0.3.jar:lib/jetty-xml-9.2.9.v20150224.jar:lib/neo4j-codegen-3.0.3.jar:lib/jackson-mapper-asl-1.9.13.jar:lib/neo4j-kernel-3.0.3.jar:lib/commons-beanutils-1.8.3.jar:lib/commons-configuration-1.10.jar:lib/neo4j-collections-3.0.3.jar:lib/lucene-queryparser-5.5.0.jar:lib/neo4j-primitive-collections-3.0.3.jar:lib/neo4j-graphdb-api-3.0.3.jar:lib/javax.servlet-api-3.1.0.jar:lib/jetty-util-9.2.9.v20150224.jar:lib/neo4j-lucene-index-3.0.3.jar:lib/opencsv-2.3.jar:lib/neo4j-cypher-frontend-3.0-3.0.3.jar:lib/lucene-analyzers-common-5.5.0.jar:lib/scala-library-2.11.8.jar:lib/rhino-1.7R4.jar:lib/neo4j-security-3.0.3.jar:lib/lucene-codecs-5.5.0.jar:lib/mimepull-1.9.3.jar:lib/neo4j-browser-1.1.6.jar:lib/jetty-http-9.2.9.v20150224.jar:lib/bcpkix-jdk15on-1.53.jar:lib/jersey-multipart-1.19.jar:lib/neo4j-udc-3.0.3.jar:lib/jersey-core-1.19.jar:lib/jetty-io-9.2.9.v20150224.jar:lib/jetty-server-9.2.9.v20150224.jar:lib/neo4j-logging-3.0.3.jar:lib/netty-all-4.0.28.Final.jar:lib/lucene-core-5.5.0.jar:lib/neo4j-io-3.0.3.jar:lib/jackson-core-asl-1.9.13.jar:lib/concurrentlinkedhashmap-lru-1.4.2.jar:lib/neo4j-cypher-compiler-2.3-2.3.4.jar:lib/scala-reflect-2.11.8.jar:lib/neo4j-bolt-3.0.3.jar:lib/neo4j-cypher-compiler-3.0-3.0.3.jar:lib/neo4j-unsafe-3.0.3.jar:lib/neo4j-cypher-3.0.3.jar:lib/jersey-server-1.19.jar:lib/neo4j-lucene-upgrade-3.0.3.jar:lib/jersey-servlet-1.19.jar";
        options[1].optionString = "-XX:+UseG1GC";
        options[2].optionString = "-XX:-OmitStackTraceInFastThrow";
        options[3].optionString = "-XX:hashCode=5";
        options[4].optionString = "-XX:+AlwaysPreTouch";
        options[5].optionString = "-XX:+UnlockExperimentalVMOptions";
        options[6].optionString = "-XX:+DisableExplicitGC";
        options[7].optionString = "-Dunsupported.dbms.udc.source=tarball";
        options[8].optionString = "-Xmx100M";
        //options[1].optionString = "-Xcheck:jni";
        vm_args.version = JNI_VERSION_1_6;
        JNI_GetDefaultJavaVMInitArgs(&vm_args);
        vm_args.nOptions = 9;
        vm_args.options = options;
        vm_args.ignoreUnrecognized = false;
        JNIEnv *env;

        jint rc = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
        initEnv(0, env);
        delete[] options;
    }

    void initInserterConfig()
    {
        envs[0].env->CallVoidMethod(hashmap, envs[0].clear);
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("cache_type"), envs[0].env->NewStringUTF("none"));
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("use_memory_mapped_buffers"), envs[0].env->NewStringUTF("true"));
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("neostore.nodestore.db.mapped_memory"), envs[0].env->NewStringUTF("200M"));
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("neostore.relationshipstore.db.mapped_memory"), envs[0].env->NewStringUTF("1000M"));
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("neostore.propertystore.db.mapped_memory"), envs[0].env->NewStringUTF("250M"));
        envs[0].env->CallObjectMethod(hashmap, envs[0].put, envs[0].env->NewStringUTF("neostore.propertystore.db.strings.mapped_memory"), envs[0].env->NewStringUTF("250M"));
    }

    void initInserter()
    {
        fprintf(stderr, "start\n");
        jclass inserters = envs[0].env->FindClass("org/neo4j/unsafe/batchinsert/BatchInserters");
        jmethodID mkinserter = envs[0].env->GetStaticMethodID(inserters, "inserter", "(Ljava/io/File;)Lorg/neo4j/unsafe/batchinsert/BatchInserter;");
        jclass fileJava = envs[0].env->FindClass("java/io/File");
        jmethodID fileInit = envs[0].env->GetMethodID(fileJava, "<init>", "(Ljava/lang/String;)V");
        jstring fileName = envs[0].env->NewStringUTF("test5.db");
        jobject fileInst = envs[0].env->NewObject(fileJava, fileInit, fileName);
        initInserterConfig();
        inserter = envs[0].env->CallObjectMethod(inserters, mkinserter, fileInst, hashmap);
        envs[0].env->CallVoidMethod(hashmap, envs[0].clear);
    }

    jobject convertToObject(struct jnienv &env, struct invariant_value value)
    {
        jobject new_val;
        switch (value.type)
        {
        case STRING :
            new_val = env.env->NewStringUTF(boost::get<std::string>(value.val).c_str());
            break;
        case BOOL :
            if (boost::get<bool>(value.val))
            {
                new_val = env.env->NewObject(env.boolean, env.boolconst, JNI_TRUE);
            }
            else
            {
                new_val = env.env->NewObject(env.boolean, env.boolconst, JNI_FALSE);
            }
            break;
        case INT :
            new_val = env.env->NewObject(env.integer, env.integerconst, boost::get<long>(value.val));
            break;
        case FLOAT :
            new_val = env.env->NewObject(env.floatc, env.floatconst, boost::get<double>(value.val));
            break;
        case BIGINT :
            new_val = env.env->NewObject(env.integer, env.integerconst, boost::get<unsigned long long int>(value.val));
            break;
        }
    }

    void fillMap(struct jnienv &env, jobject &hashmap, const std::map<std::string, struct invariant_value> &props)
    {
        env.env->CallVoidMethod(hashmap, env.clear);
        for (auto pair : props)
        {
            jstring key = env.env->NewStringUTF(pair.first.c_str());
            jobject value = convertToObject(env, pair.second);
            env.env->CallObjectMethod(hashmap, env.put, key, value);
            env.env->DeleteLocalRef(key);
            env.env->DeleteLocalRef(value);
        }
    }

    jobject makeType(struct jnienv &env, std::string type)
    {
        if (!relTypes.count(type))
        {
            jobject t = env.env->CallStaticObjectMethod(env.types, env.withname, env.env->NewStringUTF(type.c_str()));
            env.env->NewGlobalRef(t);
            relTypes.emplace(type, t);
        }
        return relTypes.at(type);
    }

public :

    Neo4jInserter()
    {
        initJVM();
        jclass mapClass = envs[0].env->FindClass("java/util/HashMap");
        jmethodID mapInit = envs[0].env->GetMethodID(mapClass, "<init>", "()V");
        hashmap = envs[0].env->NewObject(mapClass, mapInit);
        threads = 0;
        firstRel = true;
        initInserter();
        label = envs[0].env->NewStringUTF("GRAPH");
        resTrs = envs[0].env->NewStringUTF("g");
        envs[0].env->NewGlobalRef(inserter);
        envs[0].env->NewGlobalRef(hashmap);
    }

    Neo4jInserter(const Neo4jInserter &other)
    {
        envs = other.envs;
        jvm = other.jvm;
        relTypes = other.relTypes;
        inserter = other.inserter;
        hashmap = other.hashmap;
        threads = other.threads;
        firstRel = other.firstRel;
    }

    ~Neo4jInserter()
    {
        finish();
    }

    Neo4jInserter &operator=(const Neo4jInserter &other)
    {
        envs = other.envs;
        jvm = other.jvm;
        relTypes = other.relTypes;
        inserter = other.inserter;
        hashmap = other.hashmap;
        threads = other.threads;
        firstRel = other.firstRel;
    }

    int attach()
    {
        threads++;
        JNIEnv *env;
        jvm->AttachCurrentThread((void**)&env, NULL);
        initEnv(threads, env);
        return threads;
    }

    void detach(int threadNum)
    {
        //fprintf(stderr, "test %d\n", threadNum);
        jvm->DetachCurrentThread();
        struct jnienv env;
        envs[threadNum] = env;
        //fprintf(stderr, "test %d\n", threadNum);
    }

    jlong createNode(int threadNum, std::map<std::string, struct invariant_value> props)
    {
        struct jnienv env = envs[threadNum];
        fillMap(env, hashmap, props);
        jlong n = env.env->CallStaticLongMethod(env.main, env.makenode, inserter, hashmap, label);
        return n;
    }

    void addRelationship(int threadNum, jlong n1, jlong n2, std::string type, jlong g)
    {
        struct jnienv env = envs[threadNum];
        env.env->CallLongMethod(hashmap, env.clear);
        //jstring dsc1 = env.env->NewStringUTF(desc.c_str());
        //jstring ord1 = env.env->NewStringUTF(order.c_str());
        jobject new_val = env.env->NewObject(env.integer, env.integerconst, g);
        env.env->CallVoidMethod(hashmap, env.put, resTrs, new_val);
        env.env->DeleteLocalRef(new_val);
        jobject relType = makeType(env, type);
        env.env->CallLongMethod(inserter, env.createRelationship, n1, n2, relType, hashmap);
        env.env->CallLongMethod(hashmap, env.clear);
    }

    void finish()
    {
        jclass inserterClass = envs[0].env->FindClass("org/neo4j/unsafe/batchinsert/BatchInserter");
        jmethodID shutdown = envs[0].env->GetMethodID(inserterClass, "shutdown", "()V");
        envs[0].env->CallVoidMethod(inserter, shutdown);
        envs[0].env->DeleteGlobalRef(inserter);
        envs[0].env->DeleteGlobalRef(hashmap);
        for (auto elem : relTypes)
        {
            envs[0].env->DeleteGlobalRef(elem.second);
        }
        fprintf(stderr, "destroying jvm\n");
        jvm->DestroyJavaVM();
    }
};

#endif
