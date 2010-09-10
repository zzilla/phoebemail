using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.IO;
using DbService;

namespace Test
{
    class TestSuite
    {
        public static void TestMySql()
        {
            string sql = "select * from employee";
            IDataReader r = DbAccess.Instance.ExecuteReader(sql);

            while (r.Read())
            {
                int index = 0;
                Object id = r.GetValue(index++);
                Object name = r.GetValue(index++);
                Object email = r.GetValue(index++);
                Console.WriteLine("id:{0} name:{1} email:{2}", id, name, email);
            }

            DbAccess.Instance.Dispose();

            Console.ReadLine();
        }

        public static void TestExcel()
        {
            Console.WriteLine("inserting data...");

            string sql = "";
            //先插入10行数据
            for (int i = 0; i < 10; ++i)
            {
                sql = string.Format("insert into [Sheet1$](id, name, email) values('{0:00}','qguo','qguo@gmail.com')", i);

                DbAccess.Instance.ExecuteNonQuery(sql);
            }

            //更新其中后五条
            sql = string.Format("update [Sheet1$] set email = 'guoqiao@gmail.com' where id > '05'");

            DbAccess.Instance.ExecuteNonQuery(sql);

            sql = "select * from [Sheet1$]";

            IDataReader r = DbAccess.Instance.ExecuteReader(sql);

            while (r.Read())
            {
                int index = 0;
                Object id = r.GetValue(index++);
                Object name = r.GetValue(index++);
                Object email = r.GetValue(index++);
                Object internalSalary = r.GetValue(index++);
                Object externalSalary = r.GetValue(index++);
                Console.WriteLine("id:{0} name:{1} email:{2} internalSalary:{3} externalSalary:{4}", id, name, email, internalSalary, externalSalary);
            }

            DbAccess.Instance.Dispose();

            Console.ReadLine();
        }

        public static void TestText()
        {
            string file = "test.txt";

            if (!File.Exists(file))
            {
                File.Delete(file);
            }

            using (StreamWriter w = File.CreateText(file))
            {
                w.WriteLine("id,name,email");
            }

            Console.WriteLine("inserting data...");
            for (int i = 0; i < 10; ++i )
            {
                string sql0 = string.Format("insert into test.txt(id, name, email) values('{0:00}','qguo','qguo@gmail.com')", i);

                DbAccess.Instance.ExecuteNonQuery(sql0);
            }

            string sql = "select * from test.txt";

            IDataReader r = DbAccess.Instance.ExecuteReader(sql);

            while (r.Read())
            {
                int index = 0;
                Object id = r.GetValue(index++);
                Object name = r.GetValue(index++);
                Object email = r.GetValue(index++);
                Console.WriteLine("id:{0} name:{1} email:{2}", id, name, email);
            }

            //txt文件不支持update和delete, 因此用处就很小了

            sql = "select count(id) from test.txt";

            Object count = DbAccess.Instance.ExecuteScalar(sql);

            Console.WriteLine("row count: {0}", count);

            DbAccess.Instance.Dispose();
            Console.ReadLine();
        }
    }
}
