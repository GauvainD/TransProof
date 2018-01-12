#ifndef NEO4J_INSERTER_HPP
#define NEO4J_INSERTER_HPP

#include <jni.h>
#include <cstdio>
#include <map>
#include <iostream>

#include "filter_test.hpp"
#include "inserter.hpp"

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

class Neo4jInserter : public TransProofInserter
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
    jstring orderTrs;
    jstring graphLabel;
    jstring nodeLabel;
    std::string output;

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
        options[0].optionString = CPCMD;
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
        jstring fileName = envs[0].env->NewStringUTF(output.c_str());
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

    Neo4jInserter(std::string output)
    {
        this->output = output;
        initJVM();
        jclass mapClass = envs[0].env->FindClass("java/util/HashMap");
        jmethodID mapInit = envs[0].env->GetMethodID(mapClass, "<init>", "()V");
        hashmap = envs[0].env->NewObject(mapClass, mapInit);
        threads = 0;
        firstRel = true;
        initInserter();
        graphLabel = envs[0].env->NewStringUTF("GRAPH");
        nodeLabel = envs[0].env->NewStringUTF("FILTERED");
        resTrs = envs[0].env->NewStringUTF("g");
        orderTrs = envs[0].env->NewStringUTF("order");
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
        this->output = other.output;
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
        this->output = other.output;
        return *this;
    }

    int attach() override
    {
        threads++;
        JNIEnv *env;
        jvm->AttachCurrentThread((void**)&env, NULL);
        initEnv(threads, env);
        return threads;
    }

    void detach(int threadNum) override
    {
        //fprintf(stderr, "test %d\n", threadNum);
        jvm->DetachCurrentThread();
        struct jnienv env;
        envs[threadNum] = env;
        //fprintf(stderr, "test %d\n", threadNum);
    }

    jlong createNode(int threadNum, std::map<std::string, struct invariant_value> props, bool isGraph) override
    {
        struct jnienv env = envs[threadNum];
        fillMap(env, hashmap, props);
        jlong n;
        if (isGraph)
        {
            n = env.env->CallStaticLongMethod(env.main, env.makenode, inserter, hashmap, graphLabel);
        }
        else
        {
            n = env.env->CallStaticLongMethod(env.main, env.makenode, inserter, hashmap, nodeLabel);
        }
        return n;
    }

    void addRelationship(int threadNum, jlong n1, jlong n2, std::string type, jlong g, jlong order) override
    {
        struct jnienv env = envs[threadNum];
        env.env->CallLongMethod(hashmap, env.clear);
        //jstring dsc1 = env.env->NewStringUTF(desc.c_str());
        //jstring ord1 = env.env->NewStringUTF(order.c_str());
        jobject new_val = env.env->NewObject(env.integer, env.integerconst, g);
        env.env->CallVoidMethod(hashmap, env.put, resTrs, new_val);
        env.env->DeleteLocalRef(new_val);
        new_val = env.env->NewObject(env.integer, env.integerconst, order);
        env.env->CallVoidMethod(hashmap, env.put, orderTrs, new_val);
        env.env->DeleteLocalRef(new_val);
        jobject relType = makeType(env, type);
        env.env->CallLongMethod(inserter, env.createRelationship, n1, n2, relType, hashmap);
        env.env->CallLongMethod(hashmap, env.clear);
    }

    void finish() override
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
