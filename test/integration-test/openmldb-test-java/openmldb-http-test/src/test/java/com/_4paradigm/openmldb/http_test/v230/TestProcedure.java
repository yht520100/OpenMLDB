/*
 * Copyright 2021 4Paradigm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com._4paradigm.openmldb.http_test.v230;

import com._4paradigm.openmldb.http_test.common.BaseTest;
import com._4paradigm.openmldb.http_test.executor.RestfulExecutor;
import com._4paradigm.openmldb.test_common.provider.Yaml;
import com._4paradigm.openmldb.test_common.restful.model.RestfulCase;
import io.qameta.allure.Feature;
import io.qameta.allure.Story;
import org.testng.annotations.Test;

@Feature("procedure")
public class TestProcedure extends BaseTest {
    @Test(dataProvider = "getCase")
    @Yaml(filePaths = "/restful/v230/test_get_procedure.yaml")
    @Story("GetProcedure")
     public void testGet(RestfulCase restfulCase){
        new RestfulExecutor(executor,restfulCase).run();
    }

    @Test(dataProvider = "getCase")
    @Yaml(filePaths = "/restful/v230/test_execute_procedure.yaml")
    @Story("ExecuteProcedure")
    public void testExecute(RestfulCase restfulCase){
        new RestfulExecutor(executor,restfulCase).run();
    }
}
