using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Data.Common;
using System.Data.Odbc;
using System.Data.OleDb;
using System.Data.SqlClient;
using System.Data.OracleClient;
using System.IO;
using System.Xml;

namespace DbService
{
    public class DbAccess : IDisposable
    {
        private string m_invariantName = string.Empty;

        private string m_connectionString = string.Empty;

        private DbProviderFactory m_factory;

        private IDbConnection m_connection;

        private DbAccess()
        {
            PrintDbProvidersToFile();

            LoadConfigInfo();

            m_factory = DbProviderFactories.GetFactory(m_invariantName);

            m_connection = m_factory.CreateConnection();

            m_connection.ConnectionString = m_connectionString;

            m_connection.Open();
        }

        //将db提供程序信息打印到文本, 主要是方便填写配置文件
        public static void PrintDbProvidersToFile()
        {
            string file = "DbProviders.txt";

            if (File.Exists(file))
            {
                File.Delete(file);
            }

            using (StreamWriter w = File.CreateText(file))
            {
                DataTable table = DbProviderFactories.GetFactoryClasses();

                foreach (DataRow row in table.Rows)
                {
                    int index = 0;
                    w.WriteLine("Name: {0}", row.ItemArray[index]);
                    w.WriteLine("Description: {0}", row.ItemArray[++index]);
                    w.WriteLine("InvariantName: {0}", row.ItemArray[++index]);
                    w.WriteLine("AssemblyQualifiedName: {0}\n\n", row.ItemArray[++index]);
                }
            }
        }

        private void LoadConfigInfo()
        {
            string file = "DbService.xml";

            XmlDocument doc = new XmlDocument();
            doc.Load(file);

            XmlElement root = doc.DocumentElement;

            XmlNodeList list = root.GetElementsByTagName("ConnectionString");

            string dbInUse = root.Attributes["db"].Value;

            foreach (XmlNode node in list)
            {
                if (node.NodeType == XmlNodeType.Element)
                {
                    if (node.Attributes["db"].Value == dbInUse)
                    {
                        m_invariantName = node.Attributes["provider"].Value;
                        m_connectionString = node.InnerText.Trim();
                    }
                }
            } 
        }

        private static DbAccess m_instance = new DbAccess();

        public static DbAccess Instance
        {
            get { return DbAccess.m_instance; }
        }

        private IDbCommand CreateCommand(string strSql)
        {
            if (m_connection.State != ConnectionState.Open)
            {
                m_connection.Open();
            }
            IDbCommand command = m_connection.CreateCommand();
            command.CommandText = strSql;
            return command;
        }

        public IDataReader ExecuteReader(string strSql)
        {
            return CreateCommand(strSql).ExecuteReader();
        }

        //用指定的sql语句的查询结果来填充Dataset
        public DataSet ExecuteFill(string strSql)
        {
            DbDataAdapter da = m_factory.CreateDataAdapter();
            da.SelectCommand = (DbCommand)CreateCommand(strSql);
            DataSet ds = new DataSet();
            da.Fill(ds);
            return ds;
        }

        public int ExecuteNonQuery(string strSql)
        {
            return CreateCommand(strSql).ExecuteNonQuery();
        }

        public object ExecuteScalar(string strSql)
        {
            return CreateCommand(strSql).ExecuteScalar();
        }

        #region IDisposable 成员

        public void Dispose()
        {
            m_connection.Close();
        }

        #endregion
    }
}
